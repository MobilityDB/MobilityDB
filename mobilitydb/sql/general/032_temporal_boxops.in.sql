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
 * @brief Bounding tbox operators for temporal types
 */

/*****************************************************************************
 * Boxes functions
 *****************************************************************************/

CREATE FUNCTION spans(tbool)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spans(tint)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spans(tfloat)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spans(ttext)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tboxes(tint)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxes(tfloat)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitNSpans(tbool, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitNSpans(tint, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitNSpans(tfloat, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitNSpans(ttext, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitEachNSpans(tbool, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_each_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitEachNSpans(tint, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_each_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitEachNSpans(tfloat, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_each_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitEachNSpans(ttext, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_each_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitNTboxes(tint, integer)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_split_n_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitNTboxes(tfloat, integer)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_split_n_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitEachNTboxes(tint, integer)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_split_each_n_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitEachNTboxes(tfloat, integer)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_split_each_n_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Temporal boolean
 *****************************************************************************/

CREATE FUNCTION temporal_overlaps(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tstzspan, RIGHTARG = tbool,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tbool, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contains(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tstzspan, RIGHTARG = tbool,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tbool, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contained(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tstzspan, RIGHTARG = tbool,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tbool, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_same(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tstzspan, RIGHTARG = tbool,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tbool, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_adjacent(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tstzspan, RIGHTARG = tbool,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tbool, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * Temporal integer
 *****************************************************************************/

CREATE FUNCTION temporal_overlaps(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tstzspan, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tint, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_overlaps(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = intspan, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tint, RIGHTARG = intspan,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tbox, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tint, RIGHTARG = tbox,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contains(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tstzspan, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tint, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contains(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = intspan, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tint, RIGHTARG = intspan,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tbox, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tint, RIGHTARG = tbox,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contained(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tstzspan, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tint, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contained(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = intspan, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tint, RIGHTARG = intspan,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tbox, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tint, RIGHTARG = tbox,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_same(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tstzspan, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tint, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_same(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = intspan, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tint, RIGHTARG = intspan,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tbox, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tint, RIGHTARG = tbox,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_adjacent(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tstzspan, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tint, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_adjacent(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = intspan, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tint, RIGHTARG = intspan,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tbox, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tint, RIGHTARG = tbox,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Temporal float
 *****************************************************************************/

CREATE FUNCTION temporal_overlaps(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_overlaps(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_overlaps(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = floatspan, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tfloat, RIGHTARG = floatspan,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tbox, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tfloat, RIGHTARG = tbox,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contains(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contains(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = floatspan, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tfloat, RIGHTARG = floatspan,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tbox, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tfloat, RIGHTARG = tbox,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contained(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contained(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = floatspan, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tfloat, RIGHTARG = floatspan,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tbox, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tfloat, RIGHTARG = tbox,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_same(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_same(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = floatspan, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tfloat, RIGHTARG = floatspan,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tbox, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tfloat, RIGHTARG = tbox,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_adjacent(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_adjacent(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = floatspan, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tfloat, RIGHTARG = floatspan,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tbox, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tfloat, RIGHTARG = tbox,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Temporal text
 *****************************************************************************/

CREATE FUNCTION temporal_overlaps(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tstzspan, RIGHTARG = ttext,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = ttext, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contains(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tstzspan, RIGHTARG = ttext,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = ttext, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contained(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tstzspan, RIGHTARG = ttext,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = ttext, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_same(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tstzspan, RIGHTARG = ttext,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = ttext, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_adjacent(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tstzspan, RIGHTARG = ttext,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = ttext, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
