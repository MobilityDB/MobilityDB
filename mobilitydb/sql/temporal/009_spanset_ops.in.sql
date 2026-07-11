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

CREATE FUNCTION left(integer, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(intspanset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(intspanset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(intspanset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = integer, RIGHTARG = intspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = intspanset, RIGHTARG = integer,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION left(bigint, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(bigintspanset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(bigintspanset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION left(float, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(floatspanset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(floatspanset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = float, RIGHTARG = floatspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = floatspanset, RIGHTARG = float,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION before(date, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(datespan, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(datespanset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(datespanset, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(datespanset, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = date, RIGHTARG = datespanset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = datespan, RIGHTARG = datespanset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = datespanset, RIGHTARG = date,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = datespanset, RIGHTARG = datespan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION before(timestamptz, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tstzspan, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tstzspanset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tstzspanset, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tstzspanset, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION right(integer, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(intspanset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(intspanset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(intspanset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = integer, RIGHTARG = intspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = intspanset, RIGHTARG = integer,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION right(bigint, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(bigintspanset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(bigintspanset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION right(float, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(floatspanset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(floatspanset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = float, RIGHTARG = floatspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = floatspanset, RIGHTARG = float,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION after(date, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(datespan, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(datespanset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(datespanset, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(datespanset, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = date, RIGHTARG = datespanset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = datespan, RIGHTARG = datespanset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = datespanset, RIGHTARG = date,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = datespanset, RIGHTARG = datespan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION after(timestamptz, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzspan, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzspanset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzspanset, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzspanset, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION overleft(integer, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(intspanset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(intspanset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(intspanset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = integer, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = intspan, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = intspanset, RIGHTARG = integer,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = intspanset, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overleft(bigint, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(bigintspanset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(bigintspanset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overleft(float, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(floatspanset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(floatspanset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = float, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = floatspanset, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overbefore(date, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(datespan, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(datespanset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(datespanset, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(datespanset, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = date, RIGHTARG = datespanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = datespan, RIGHTARG = datespanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = datespanset, RIGHTARG = date,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = datespanset, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overbefore(timestamptz, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzspan, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzspanset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzspanset, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzspanset, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION overright(integer, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(intspanset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(intspanset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(intspanset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = integer, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = intspan, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = intspanset, RIGHTARG = integer,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = intspanset, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overright(bigint, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(bigintspanset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(bigintspanset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overright(float, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(floatspanset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(floatspanset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = float, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = floatspanset, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overafter(date, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(datespan, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(datespanset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(datespanset, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(datespanset, datespanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = date, RIGHTARG = datespanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = datespan, RIGHTARG = datespanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = datespanset, RIGHTARG = date,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = datespanset, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overafter(timestamptz, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzspan, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzspanset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzspanset, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzspanset, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
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

CREATE FUNCTION spansetUnion(integer, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(intspan, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(intspanset, integer)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(intspanset, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(intspanset, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = integer, RIGHTARG = intspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = intspanset, RIGHTARG = integer,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = +
);

CREATE FUNCTION spansetUnion(bigint, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(bigintspan, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(bigintspanset, bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(bigintspanset, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(bigintspanset, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = +
);

CREATE FUNCTION spansetUnion(float, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(floatspan, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(floatspanset, float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(floatspanset, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(floatspanset, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = float, RIGHTARG = floatspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = floatspanset, RIGHTARG = float,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = +
);

CREATE FUNCTION spansetUnion(date, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(datespan, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(datespanset, date)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(datespanset, datespan)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(datespanset, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = date, RIGHTARG = datespanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = datespan, RIGHTARG = datespanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = datespanset, RIGHTARG = date,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = datespanset, RIGHTARG = datespan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  COMMUTATOR = +
);

CREATE FUNCTION spansetUnion(timestamptz, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(tstzspan, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(tstzspanset, timestamptz)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(tstzspanset, tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetUnion(tstzspanset, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spansetUnion,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  COMMUTATOR = +
);

/*****************************************************************************/

CREATE FUNCTION spansetMinus(integer, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(intspan, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(intspanset, integer)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(intspanset, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(intspanset, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = integer, RIGHTARG = intspanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = intspan, RIGHTARG = intspanset
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = intspanset, RIGHTARG = integer
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = intspanset, RIGHTARG = intspan
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = intspanset, RIGHTARG = intspanset
);

CREATE FUNCTION spansetMinus(bigint, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(bigintspan, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(bigintspanset, bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(bigintspanset, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(bigintspanset, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = bigint, RIGHTARG = bigintspanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = bigintspanset, RIGHTARG = bigint
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset
);

CREATE FUNCTION spansetMinus(float, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(floatspan, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(floatspanset, float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(floatspanset, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(floatspanset, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = float, RIGHTARG = floatspanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = floatspan, RIGHTARG = floatspanset
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = floatspanset, RIGHTARG = float
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = floatspanset, RIGHTARG = floatspan
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = floatspanset, RIGHTARG = floatspanset
);

CREATE FUNCTION spansetMinus(date, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(datespan, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(datespanset, date)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(datespanset, datespan)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(datespanset, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = date, RIGHTARG = datespanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = datespan, RIGHTARG = datespanset
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = datespanset, RIGHTARG = date
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = datespanset, RIGHTARG = datespan
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = datespanset, RIGHTARG = datespanset
);

CREATE FUNCTION spansetMinus(timestamptz, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(tstzspan, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(tstzspanset, timestamptz)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(tstzspanset, tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetMinus(tstzspanset, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan
);
CREATE OPERATOR - (
  PROCEDURE = spansetMinus,
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset
);

/*****************************************************************************/

CREATE FUNCTION spansetIntersection(integer, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(intspan, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(intspanset, integer)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(intspanset, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(intspanset, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = integer, RIGHTARG = intspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = intspanset, RIGHTARG = integer,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = *
);

CREATE FUNCTION spansetIntersection(bigint, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(bigintspan, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(bigintspanset, bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(bigintspanset, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(bigintspanset, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = *
);

CREATE FUNCTION spansetIntersection(float, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(floatspan, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(floatspanset, float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(floatspanset, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(floatspanset, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = float, RIGHTARG = floatspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = floatspanset, RIGHTARG = float,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = *
);

CREATE FUNCTION spansetIntersection(date, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Intersection_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(datespan, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Intersection_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(datespanset, date)
  RETURNS date
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(datespanset, datespan)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(datespanset, datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = date, RIGHTARG = datespanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = datespan, RIGHTARG = datespanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = datespanset, RIGHTARG = date,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = datespanset, RIGHTARG = datespan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = datespanset, RIGHTARG = datespanset,
  COMMUTATOR = *
);

CREATE FUNCTION spansetIntersection(timestamptz, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Intersection_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(tstzspan, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Intersection_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(tstzspanset, timestamptz)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(tstzspanset, tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetIntersection(tstzspanset, tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = timestamptz, RIGHTARG = tstzspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = tstzspan, RIGHTARG = tstzspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = tstzspanset, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
  LEFTARG = tstzspanset, RIGHTARG = tstzspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spansetIntersection,
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
