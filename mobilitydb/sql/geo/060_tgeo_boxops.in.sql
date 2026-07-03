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
 * @brief Bounding box operators for temporal geometries/geographies
 */

/*****************************************************************************
 * Conversions
 *****************************************************************************/

CREATE FUNCTION stbox(tgeometry)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tspatial_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(tgeography)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tspatial_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeometry AS stbox) WITH FUNCTION stbox(tgeometry);
CREATE CAST (tgeography AS stbox) WITH FUNCTION stbox(tgeography);

/*****************************************************************************/

CREATE FUNCTION expandSpace(tgeometry, float)
  RETURNS stbox
  AS 'SELECT @extschema@.expandSpace($1::stbox, $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION expandSpace(tgeography, float)
  RETURNS stbox
  AS 'SELECT @extschema@.expandSpace($1::stbox, $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/

CREATE FUNCTION spans(tgeometry)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spans(tgeography)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stboxes(tgeometry)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Tgeo_stboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxes(tgeography)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Tgeo_stboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION splitNSpans(tgeometry, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitNSpans(tgeography, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitEachNSpans(tgeometry, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_each_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitEachNSpans(tgeography, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_each_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitNStboxes(tgeometry, integer)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Tgeo_split_n_stboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitNStboxes(tgeography, integer)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Tgeo_split_n_stboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitEachNStboxes(tgeometry, integer)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Tgeo_split_each_n_stboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitEachNStboxes(tgeography, integer)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Tgeo_split_each_n_stboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Contains
 *****************************************************************************/

CREATE FUNCTION contains(tstzspan, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(tgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tstzspan, RIGHTARG = tgeometry,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tgeometry, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains(tstzspan, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(tgeography, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tstzspan, RIGHTARG = tgeography,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tgeography, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = stbox, RIGHTARG = tgeometry,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tgeometry, RIGHTARG = stbox,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains(stbox, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(tgeography, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = stbox, RIGHTARG = tgeography,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tgeography, RIGHTARG = stbox,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tgeography, RIGHTARG = tgeography,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Contained
 *****************************************************************************/

CREATE FUNCTION contained(tstzspan, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(tgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = tstzspan, RIGHTARG = tgeometry,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = tgeometry, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = stbox, RIGHTARG = tgeometry,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = tgeometry, RIGHTARG = stbox,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained(tstzspan, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(tgeography, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = tstzspan, RIGHTARG = tgeography,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = tgeography, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained(stbox, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(tgeography, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = stbox, RIGHTARG = tgeography,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = tgeography, RIGHTARG = stbox,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = tgeography, RIGHTARG = tgeography,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

CREATE FUNCTION overlaps(tstzspan, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(tgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = tstzspan, RIGHTARG = tgeometry,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = tgeometry, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION overlaps(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = stbox, RIGHTARG = tgeometry,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = tgeometry, RIGHTARG = stbox,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION overlaps(tstzspan, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(tgeography, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = tstzspan, RIGHTARG = tgeography,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = tgeography, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION overlaps(stbox, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(tgeography, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = stbox, RIGHTARG = tgeography,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = tgeography, RIGHTARG = stbox,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = tgeography, RIGHTARG = tgeography,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION same(tstzspan, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same(tgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = tstzspan, RIGHTARG = tgeometry,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = tgeometry, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = stbox, RIGHTARG = tgeometry,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = tgeometry, RIGHTARG = stbox,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same(tstzspan, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same(tgeography, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = tstzspan, RIGHTARG = tgeography,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = tgeography, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same(stbox, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same(tgeography, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = stbox, RIGHTARG = tgeography,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = tgeography, RIGHTARG = stbox,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = tgeography, RIGHTARG = tgeography,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Adjacent
 *****************************************************************************/

CREATE FUNCTION adjacent(tstzspan, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent(tgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = tstzspan, RIGHTARG = tgeometry,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = tgeometry, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = stbox, RIGHTARG = tgeometry,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = tgeometry, RIGHTARG = stbox,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent(tstzspan, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent(tgeography, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = tstzspan, RIGHTARG = tgeography,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = tgeography, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent(stbox, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent(tgeography, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = stbox, RIGHTARG = tgeography,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = tgeography, RIGHTARG = stbox,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = tgeography, RIGHTARG = tgeography,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/
