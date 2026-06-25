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
 * @brief Operators for span set types
 */

CREATE FUNCTION tprecision(timestamptz, duration interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Timestamptz_tprecision'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION tprecision(tstzset, duration interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Tstzset_tprecision'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION tprecision(tstzspan, duration interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Tstzspan_tprecision'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION tprecision(tstzspanset, duration interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Tstzspanset_tprecision'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION span_contains(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(intspanset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_value'
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
  LEFTARG = intspanset, RIGHTARG = integer,
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

CREATE FUNCTION span_contains(datespan, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(datespanset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(datespanset, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(datespanset, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = datespan, RIGHTARG = datespanset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = datespanset, RIGHTARG = date,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = datespanset, RIGHTARG = datespan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contains(tstzspan, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(tstzspanset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(tstzspanset, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(tstzspanset, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_contained(integer, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_spanset'
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
  LEFTARG = integer, RIGHTARG = intspanset,
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

CREATE FUNCTION span_contained(date, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(datespan, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(datespanset, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(datespanset, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = date, RIGHTARG = datespanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = datespan, RIGHTARG = datespanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = datespanset, RIGHTARG = datespan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contained(timestamptz, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(tstzspan, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(tstzspanset, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(tstzspanset, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_overlaps(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_spanset'
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
  LEFTARG = intspan, RIGHTARG = intspanset,
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

CREATE FUNCTION span_overlaps(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_spanset'
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
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
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

CREATE FUNCTION span_overlaps(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_spanset'
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
  LEFTARG = floatspan, RIGHTARG = floatspanset,
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

CREATE FUNCTION span_overlaps(datespan, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(datespanset, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(datespanset, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = datespan, RIGHTARG = datespanset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = datespanset, RIGHTARG = datespan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overlaps(tstzspan, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(tstzspanset, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(tstzspanset, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_left(integer, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(intspanset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_value'
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
  LEFTARG = integer, RIGHTARG = intspanset,
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
  LEFTARG = intspanset, RIGHTARG = integer,
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
CREATE FUNCTION span_left(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_value'
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
CREATE FUNCTION span_left(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_value'
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

CREATE FUNCTION span_left(date, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(datespan, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(datespanset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(datespanset, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(datespanset, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = date, RIGHTARG = datespanset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = datespan, RIGHTARG = datespanset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = datespanset, RIGHTARG = date,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = datespanset, RIGHTARG = datespan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_left(timestamptz, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(tstzspan, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(tstzspanset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(tstzspanset, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(tstzspanset, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_right(integer, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(intspanset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_value'
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
  LEFTARG = integer, RIGHTARG = intspanset,
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
  LEFTARG = intspanset, RIGHTARG = integer,
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
CREATE FUNCTION span_right(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_value'
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
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_right(float, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_value'
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

CREATE FUNCTION span_right(date, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(datespan, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(datespanset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(datespanset, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(datespanset, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = date, RIGHTARG = datespanset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = datespan, RIGHTARG = datespanset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = datespanset, RIGHTARG = date,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = datespanset, RIGHTARG = datespan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_right(timestamptz, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(tstzspan, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(tstzspanset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(tstzspanset, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(tstzspanset, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_overleft(integer, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(intspanset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_value'
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
  LEFTARG = integer, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = intspan, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = intspanset, RIGHTARG = integer,
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
CREATE FUNCTION span_overleft(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_value'
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
CREATE FUNCTION span_overleft(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_value'
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
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overleft(date, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(datespan, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(datespanset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(datespanset, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(datespanset, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = date, RIGHTARG = datespanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = datespan, RIGHTARG = datespanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = datespanset, RIGHTARG = date,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = datespanset, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overleft(timestamptz, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(tstzspan, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(tstzspanset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(tstzspanset, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(tstzspanset, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_overright(integer, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(intspanset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_value'
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
  LEFTARG = integer, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = intspan, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = intspanset, RIGHTARG = integer,
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
CREATE FUNCTION span_overright(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_value'
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
CREATE FUNCTION span_overright(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_value'
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
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overright(date, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(datespan, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(datespanset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(datespanset, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(datespanset, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = date, RIGHTARG = datespanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = datespan, RIGHTARG = datespanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = datespanset, RIGHTARG = date,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = datespanset, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overright(timestamptz, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(tstzspan, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(tstzspanset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(tstzspanset, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(tstzspanset, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_adjacent(integer, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(intspanset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_value'
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
  LEFTARG = integer, RIGHTARG = intspanset,
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
  LEFTARG = intspanset, RIGHTARG = integer,
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
CREATE FUNCTION span_adjacent(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_value'
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
CREATE FUNCTION span_adjacent(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_value'
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

CREATE FUNCTION span_adjacent(date, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(datespan, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(datespanset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(datespanset, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(datespanset, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = date, RIGHTARG = datespanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = datespan, RIGHTARG = datespanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = datespanset, RIGHTARG = date,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = datespanset, RIGHTARG = datespan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_adjacent(timestamptz, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(tstzspan, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(tstzspanset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(tstzspanset, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(tstzspanset, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION spanUnion(integer, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(intspan, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(intspanset, integer)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(intspanset, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(intspanset, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = integer, RIGHTARG = intspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = intspanset, RIGHTARG = integer,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = +
);

CREATE FUNCTION spanUnion(bigint, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(bigintspan, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(bigintspanset, bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(bigintspanset, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(bigintspanset, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = +
);

CREATE FUNCTION spanUnion(float, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(floatspan, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(floatspanset, float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(floatspanset, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(floatspanset, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = float, RIGHTARG = floatspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = floatspanset, RIGHTARG = float,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = +
);

CREATE FUNCTION spanUnion(date, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(datespan, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(datespanset, date)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(datespanset, datespan)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(datespanset, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = date, RIGHTARG = datespanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = datespan, RIGHTARG = datespanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = datespanset, RIGHTARG = date,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = datespanset, RIGHTARG = datespan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  COMMUTATOR = +
);

CREATE FUNCTION spanUnion(timestamptz, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(tstzspan, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(tstzspanset, timestamptz)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(tstzspanset, tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(tstzspanset, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  COMMUTATOR = +
);

/*****************************************************************************/

CREATE FUNCTION spanMinus(integer, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(intspan, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(intspanset, integer)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(intspanset, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(intspanset, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = integer, RIGHTARG = intspanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = intspan, RIGHTARG = intspanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = intspanset, RIGHTARG = integer
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = intspanset, RIGHTARG = intspan
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = intspanset, RIGHTARG = intspanset
);

CREATE FUNCTION spanMinus(bigint, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(bigintspan, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(bigintspanset, bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(bigintspanset, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(bigintspanset, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = bigint, RIGHTARG = bigintspanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = bigintspanset, RIGHTARG = bigint
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset
);

CREATE FUNCTION spanMinus(float, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(floatspan, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(floatspanset, float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(floatspanset, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(floatspanset, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = float, RIGHTARG = floatspanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = floatspan, RIGHTARG = floatspanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = floatspanset, RIGHTARG = float
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = floatspanset, RIGHTARG = floatspan
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = floatspanset, RIGHTARG = floatspanset
);

CREATE FUNCTION spanMinus(date, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(datespan, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(datespanset, date)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(datespanset, datespan)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(datespanset, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = date, RIGHTARG = datespanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = datespan, RIGHTARG = datespanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = datespanset, RIGHTARG = date
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = datespanset, RIGHTARG = datespan
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = datespanset, RIGHTARG = datespanset
);

CREATE FUNCTION spanMinus(timestamptz, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(tstzspan, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(tstzspanset, timestamptz)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(tstzspanset, tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(tstzspanset, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset
);

/*****************************************************************************/

CREATE FUNCTION spanIntersection(integer, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(intspan, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(intspanset, integer)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(intspanset, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(intspanset, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = integer, RIGHTARG = intspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = intspanset, RIGHTARG = integer,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = *
);

CREATE FUNCTION spanIntersection(bigint, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(bigintspan, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(bigintspanset, bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(bigintspanset, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(bigintspanset, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = *
);

CREATE FUNCTION spanIntersection(float, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(floatspan, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(floatspanset, float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(floatspanset, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(floatspanset, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = float, RIGHTARG = floatspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = floatspanset, RIGHTARG = float,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = *
);

CREATE FUNCTION spanIntersection(date, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Intersection_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(datespan, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Intersection_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(datespanset, date)
  RETURNS date
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(datespanset, datespan)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(datespanset, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = date, RIGHTARG = datespanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = datespan, RIGHTARG = datespanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = datespanset, RIGHTARG = date,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = datespanset, RIGHTARG = datespan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  COMMUTATOR = *
);

CREATE FUNCTION spanIntersection(timestamptz, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Intersection_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(tstzspan, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Intersection_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(tstzspanset, timestamptz)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(tstzspanset, tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(tstzspanset, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  COMMUTATOR = *
);

/*****************************************************************************
 * Distance operators
 *****************************************************************************/

CREATE FUNCTION span_distance(integer, intspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(intspan, intspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(intspanset, integer)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(intspanset, intspan)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(intspanset, intspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = integer, RIGHTARG = intspanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = intspanset, RIGHTARG = integer,
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
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(bigintspan, bigintspanset)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(bigintspanset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(bigintspanset, bigintspan)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(bigintspanset, bigintspanset)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
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
CREATE FUNCTION span_distance(floatspan, floatspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(floatspanset, float)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_value'
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
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = <->
);

CREATE FUNCTION span_distance(date, datespanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(datespan, datespanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(datespanset, date)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(datespanset, datespan)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(datespanset, datespanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = date, RIGHTARG = datespanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = datespan, RIGHTARG = datespanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = datespanset, RIGHTARG = date,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = datespanset, RIGHTARG = datespan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  COMMUTATOR = <->
);

CREATE FUNCTION time_distance(timestamptz, tstzspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(tstzspan, tstzspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(tstzspanset, timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(tstzspanset, tstzspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(tstzspanset, tstzspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  COMMUTATOR = <->
);

/*****************************************************************************/
