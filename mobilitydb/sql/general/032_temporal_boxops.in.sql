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
 * temporal_boxops.sql
 * Bounding tbox operators for temporal types.
 */

/*****************************************************************************
 * Temporal boolean
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(timestamptz, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tbool, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tbool, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzspanset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tbool, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = timestamptz, RIGHTARG = tbool,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tbool, RIGHTARG = timestamptz,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzset, RIGHTARG = tbool,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tbool, RIGHTARG = tstzset,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzspan, RIGHTARG = tbool,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tbool, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tbool,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tbool, RIGHTARG = tstzspanset,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(timestamptz, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbool, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbool, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzspanset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbool, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = timestamptz, RIGHTARG = tbool,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tbool, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzset, RIGHTARG = tbool,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tbool, RIGHTARG = tstzset,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzspan, RIGHTARG = tbool,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tbool, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tbool,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tbool, RIGHTARG = tstzspanset,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(timestamptz, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbool, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbool, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzspanset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbool, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = timestamptz, RIGHTARG = tbool,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tbool, RIGHTARG = timestamptz,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzset, RIGHTARG = tbool,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tbool, RIGHTARG = tstzset,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzspan, RIGHTARG = tbool,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tbool, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tbool,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tbool, RIGHTARG = tstzspanset,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(timestamptz, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbool, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbool, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzspanset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbool, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = timestamptz, RIGHTARG = tbool,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tbool, RIGHTARG = timestamptz,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzset, RIGHTARG = tbool,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tbool, RIGHTARG = tstzset,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzspan, RIGHTARG = tbool,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tbool, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tbool,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tbool, RIGHTARG = tstzspanset,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent_bbox(timestamptz, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tbool, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tbool, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzspanset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tbool, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = timestamptz, RIGHTARG = tbool,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tbool, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzset, RIGHTARG = tbool,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tbool, RIGHTARG = tstzset,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzspan, RIGHTARG = tbool,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tbool, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tbool,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tbool, RIGHTARG = tstzspanset,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * Temporal integer
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(timestamptz, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = timestamptz, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tint, RIGHTARG = timestamptz,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzset, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tint, RIGHTARG = tstzset,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzspan, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tint, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tint, RIGHTARG = tstzspanset,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION overlaps_bbox(int, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(float, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(intspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = int, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tint, RIGHTARG = int,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = intspan, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tint, RIGHTARG = intspan,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = intspanset, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tint, RIGHTARG = intspanset,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tbox, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tint, RIGHTARG = tbox,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(timestamptz, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = timestamptz, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tint, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzset, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tint, RIGHTARG = tstzset,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzspan, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tint, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tint, RIGHTARG = tstzspanset,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(int, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(float, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(intspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = int, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tint, RIGHTARG = int,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = intspan, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tint, RIGHTARG = intspan,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = intspanset, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tint, RIGHTARG = intspanset,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tbox, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tint, RIGHTARG = tbox,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(timestamptz, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = timestamptz, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tint, RIGHTARG = timestamptz,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzset, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tint, RIGHTARG = tstzset,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzspan, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tint, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tint, RIGHTARG = tstzspanset,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(int, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(float, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(intspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = int, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tint, RIGHTARG = int,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = intspan, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tint, RIGHTARG = intspan,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = intspanset, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tint, RIGHTARG = intspanset,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tbox, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tint, RIGHTARG = tbox,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(timestamptz, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = timestamptz, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tint, RIGHTARG = timestamptz,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzset, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tint, RIGHTARG = tstzset,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzspan, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tint, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tint, RIGHTARG = tstzspanset,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(int, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(float, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(intspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = int, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tint, RIGHTARG = int,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = intspan, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tint, RIGHTARG = intspan,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = intspanset, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tint, RIGHTARG = intspanset,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tbox, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tint, RIGHTARG = tbox,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent_bbox(timestamptz, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = timestamptz, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzset, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = tstzset,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzspan, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = tstzspanset,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent_bbox(int, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(float, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(intspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = int, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = int,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = intspan, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = intspan,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = intspanset, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = intspanset,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tbox, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = tbox,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Temporal float
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(timestamptz, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = timestamptz, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tfloat, RIGHTARG = timestamptz,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzset, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzset,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzspanset,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION overlaps_bbox(int, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(floatspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = int, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tfloat, RIGHTARG = int,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = floatspan, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tfloat, RIGHTARG = floatspan,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = floatspanset, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tfloat, RIGHTARG = floatspanset,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tbox, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tfloat, RIGHTARG = tbox,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(timestamptz, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = timestamptz, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tfloat, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzset, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzset,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzspanset,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(int, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(floatspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = int, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tfloat, RIGHTARG = int,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = floatspan, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tfloat, RIGHTARG = floatspan,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = floatspanset, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tfloat, RIGHTARG = floatspanset,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tbox, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tfloat, RIGHTARG = tbox,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(timestamptz, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = timestamptz, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tfloat, RIGHTARG = timestamptz,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzset, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzset,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzspanset,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(int, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(floatspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = int, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tfloat, RIGHTARG = int,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = floatspan, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tfloat, RIGHTARG = floatspan,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = floatspanset, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tfloat, RIGHTARG = floatspanset,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tbox, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tfloat, RIGHTARG = tbox,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(timestamptz, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = timestamptz, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tfloat, RIGHTARG = timestamptz,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzset, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzset,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzspanset,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(int, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(floatspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = int, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tfloat, RIGHTARG = int,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = floatspan, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tfloat, RIGHTARG = floatspan,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = floatspanset, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tfloat, RIGHTARG = floatspanset,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tbox, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tfloat, RIGHTARG = tbox,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent_bbox(timestamptz, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = timestamptz, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzset, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzset,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = tstzspanset,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent_bbox(int, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(floatspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = int, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = int,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = floatspan, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = floatspan,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = floatspanset, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = floatspanset,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tbox, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = tbox,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Temporal text
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(timestamptz, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(ttext, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(ttext, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzspanset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(ttext, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = timestamptz, RIGHTARG = ttext,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = ttext, RIGHTARG = timestamptz,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzset, RIGHTARG = ttext,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = ttext, RIGHTARG = tstzset,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzspan, RIGHTARG = ttext,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = ttext, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzspanset, RIGHTARG = ttext,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = ttext, RIGHTARG = tstzspanset,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(timestamptz, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(ttext, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(ttext, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzspanset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(ttext, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = timestamptz, RIGHTARG = ttext,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = ttext, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzset, RIGHTARG = ttext,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = ttext, RIGHTARG = tstzset,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzspan, RIGHTARG = ttext,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = ttext, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzspanset, RIGHTARG = ttext,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = ttext, RIGHTARG = tstzspanset,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(timestamptz, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(ttext, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(ttext, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzspanset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(ttext, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = timestamptz, RIGHTARG = ttext,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = ttext, RIGHTARG = timestamptz,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzset, RIGHTARG = ttext,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = ttext, RIGHTARG = tstzset,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzspan, RIGHTARG = ttext,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = ttext, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzspanset, RIGHTARG = ttext,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = ttext, RIGHTARG = tstzspanset,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(timestamptz, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(ttext, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(ttext, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzspanset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(ttext, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = timestamptz, RIGHTARG = ttext,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = ttext, RIGHTARG = timestamptz,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzset, RIGHTARG = ttext,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = ttext, RIGHTARG = tstzset,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzspan, RIGHTARG = ttext,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = ttext, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzspanset, RIGHTARG = ttext,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = ttext, RIGHTARG = tstzspanset,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent_bbox(timestamptz, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(ttext, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(ttext, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzspanset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(ttext, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = timestamptz, RIGHTARG = ttext,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = ttext, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzset, RIGHTARG = ttext,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = ttext, RIGHTARG = tstzset,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzspan, RIGHTARG = ttext,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = ttext, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzspanset, RIGHTARG = ttext,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = ttext, RIGHTARG = tstzspanset,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
