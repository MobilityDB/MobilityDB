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

/**
 * tnpoint_boxops.sql
 * Bounding box operators for temporal network points.
 */

/*****************************************************************************/

CREATE FUNCTION tnpoint_sel(internal, oid, internal, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Tnpoint_sel'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnpoint_joinsel(internal, oid, internal, smallint, internal)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Tnpoint_joinsel'
  LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************
 * Temporal npoint to stbox
 *****************************************************************************/

CREATE FUNCTION stbox(npoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Npoint_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(nsegment)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Nsegment_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(npoint, timestamptz)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Npoint_timestamp_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(npoint, tstzspan)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Npoint_period_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(tnpoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tnpoint_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (npoint AS stbox) WITH FUNCTION stbox(npoint);
CREATE CAST (nsegment AS stbox) WITH FUNCTION stbox(nsegment);
CREATE CAST (tnpoint AS stbox) WITH FUNCTION stbox(tnpoint);

/*****************************************************************************/

CREATE FUNCTION expandSpace(tnpoint, float)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tpoint_expand_space'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Contains
 *****************************************************************************/

CREATE FUNCTION temporal_contains(tstzspan, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tnpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tstzspan, RIGHTARG = tnpoint,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tnpoint, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contains(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = stbox, RIGHTARG = tnpoint,
  COMMUTATOR = <@,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

CREATE FUNCTION temporal_contains(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tnpoint, RIGHTARG = stbox,
  COMMUTATOR = <@,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = <@,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

/*****************************************************************************
 * Contained
 *****************************************************************************/

CREATE FUNCTION temporal_contained(tstzspan, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tnpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tstzspan, RIGHTARG = tnpoint,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tnpoint, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contained(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = stbox, RIGHTARG = tnpoint,
  COMMUTATOR = @>,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

CREATE FUNCTION temporal_contained(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tnpoint, RIGHTARG = stbox,
  COMMUTATOR = @>,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = @>,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

CREATE FUNCTION temporal_overlaps(tstzspan, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tnpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tstzspan, RIGHTARG = tnpoint,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tnpoint, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_overlaps(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = stbox, RIGHTARG = tnpoint,
  COMMUTATOR = &&,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

CREATE FUNCTION temporal_overlaps(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tnpoint, RIGHTARG = stbox,
  COMMUTATOR = &&,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = &&,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION temporal_same(tstzspan, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tnpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tstzspan, RIGHTARG = tnpoint,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tnpoint, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_same(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = stbox, RIGHTARG = tnpoint,
  COMMUTATOR = ~=,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

CREATE FUNCTION temporal_same(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tnpoint, RIGHTARG = stbox,
  COMMUTATOR = ~=,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = ~=,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

/*****************************************************************************
 * adjacent
 *****************************************************************************/

CREATE FUNCTION temporal_adjacent(tstzspan, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tnpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tstzspan, RIGHTARG = tnpoint,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tnpoint, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_adjacent(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = stbox, RIGHTARG = tnpoint,
  COMMUTATOR = -|-,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

CREATE FUNCTION temporal_adjacent(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tnpoint, RIGHTARG = stbox,
  COMMUTATOR = -|-,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = -|-,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

/*****************************************************************************/
