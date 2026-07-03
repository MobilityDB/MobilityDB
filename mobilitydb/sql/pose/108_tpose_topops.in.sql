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
 * @brief Bounding box operators for temporal poses
 */

/*****************************************************************************
 * Temporal pose to stbox
 *****************************************************************************/

CREATE FUNCTION stbox(pose)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Pose_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(pose, timestamptz)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Pose_timestamptz_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(pose, tstzspan)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Pose_tstzspan_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(tpose)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tspatial_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (pose AS stbox) WITH FUNCTION stbox(pose);
CREATE CAST (tpose AS stbox) WITH FUNCTION stbox(tpose);

/*****************************************************************************/

CREATE FUNCTION stboxes(tpose)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Tgeo_stboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitNStboxes(tpose, integer)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Tgeo_split_n_stboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitEachNStboxes(tpose, integer)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Tgeo_split_each_n_stboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION expandSpace(tpose, float)
  RETURNS stbox
  AS 'SELECT @extschema@.expandSpace($1::stbox, $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************
 * Contains
 *****************************************************************************/

CREATE FUNCTION contains(tstzspan, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(tpose, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tstzspan, RIGHTARG = tpose,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tpose, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains(stbox, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = stbox, RIGHTARG = tpose,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION contains(tpose, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(tpose, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tpose, RIGHTARG = stbox,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tpose, RIGHTARG = tpose,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Contained
 *****************************************************************************/

CREATE FUNCTION contained(tstzspan, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(tpose, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = tstzspan, RIGHTARG = tpose,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = tpose, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained(stbox, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = stbox, RIGHTARG = tpose,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION contained(tpose, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(tpose, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = tpose, RIGHTARG = stbox,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = tpose, RIGHTARG = tpose,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

CREATE FUNCTION overlaps(tstzspan, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(tpose, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = tstzspan, RIGHTARG = tpose,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = tpose, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION overlaps(stbox, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = stbox, RIGHTARG = tpose,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION overlaps(tpose, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(tpose, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = tpose, RIGHTARG = stbox,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = tpose, RIGHTARG = tpose,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION same(tstzspan, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same(tpose, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = tstzspan, RIGHTARG = tpose,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = tpose, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same(stbox, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = stbox, RIGHTARG = tpose,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION same(tpose, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same(tpose, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = tpose, RIGHTARG = stbox,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = tpose, RIGHTARG = tpose,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * adjacent
 *****************************************************************************/

CREATE FUNCTION adjacent(tstzspan, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent(tpose, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = tstzspan, RIGHTARG = tpose,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = tpose, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent(stbox, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = stbox, RIGHTARG = tpose,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION adjacent(tpose, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent(tpose, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = tpose, RIGHTARG = stbox,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = tpose, RIGHTARG = tpose,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/
