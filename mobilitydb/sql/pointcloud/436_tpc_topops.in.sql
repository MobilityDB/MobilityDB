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
 * @brief Bounding box operators of the temporal pgpointcloud types
 * @details Bounding-box operators for tpcpoint and tpcpatch — overlaps
 * (&&), contains (\@>), contained (<\@), same (~=), and adjacent
 * (-|-) — paired against tpcbox, tstzspan, and the temporal type
 * itself. Mirrors the cbuffer / npoint topops surface.
 *
 * tstzspan-paired variants reuse the generic `Overlaps_tstzspan_temporal`
 * et al. PG functions (already exist for every temporal type); only
 * the SQL-level CREATE FUNCTION + CREATE OPERATOR declarations are
 * needed.
 *
 * tpcbox- and tpointcloud-paired variants use the type-specific PG
 * wrappers from `mobilitydb/src/pointcloud/tpc_boxops.c`.
 */

/*****************************************************************************
 * Contains
 *****************************************************************************/

CREATE FUNCTION tpcboxContains(tstzspan, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxContains(tpcpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = tpcboxContains,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = tpcboxContains,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxContains(tpcbox, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpcbox_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxContains(tpcpoint, tpcbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpointcloud_tpcbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxContains(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpointcloud_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = tpcboxContains,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = tpcboxContains,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = tpcboxContains,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxContains(tstzspan, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxContains(tpcpatch, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = tpcboxContains,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = tpcboxContains,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxContains(tpcbox, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpcbox_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxContains(tpcpatch, tpcbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpointcloud_tpcbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxContains(tpcpatch, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpointcloud_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = tpcboxContains,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = tpcboxContains,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = tpcboxContains,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Contained
 *****************************************************************************/

CREATE FUNCTION tpcboxContained(tstzspan, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxContained(tpcpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = tpcboxContained,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tpcboxContained,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxContained(tpcbox, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpcbox_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxContained(tpcpoint, tpcbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpointcloud_tpcbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxContained(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpointcloud_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = tpcboxContained,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tpcboxContained,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tpcboxContained,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxContained(tstzspan, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxContained(tpcpatch, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = tpcboxContained,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tpcboxContained,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxContained(tpcbox, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpcbox_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxContained(tpcpatch, tpcbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpointcloud_tpcbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxContained(tpcpatch, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpointcloud_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = tpcboxContained,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tpcboxContained,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tpcboxContained,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

CREATE FUNCTION tpcboxOverlaps(tstzspan, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverlaps(tpcpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = tpcboxOverlaps,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tpcboxOverlaps,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxOverlaps(tpcbox, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpcbox_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverlaps(tpcpoint, tpcbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpointcloud_tpcbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverlaps(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpointcloud_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = tpcboxOverlaps,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tpcboxOverlaps,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tpcboxOverlaps,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxOverlaps(tstzspan, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverlaps(tpcpatch, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = tpcboxOverlaps,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tpcboxOverlaps,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxOverlaps(tpcbox, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpcbox_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverlaps(tpcpatch, tpcbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpointcloud_tpcbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverlaps(tpcpatch, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpointcloud_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = tpcboxOverlaps,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tpcboxOverlaps,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tpcboxOverlaps,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION tpcboxSame(tstzspan, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxSame(tpcpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = tpcboxSame,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tpcboxSame,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxSame(tpcbox, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpcbox_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxSame(tpcpoint, tpcbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpointcloud_tpcbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxSame(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpointcloud_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = tpcboxSame,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tpcboxSame,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tpcboxSame,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxSame(tstzspan, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxSame(tpcpatch, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = tpcboxSame,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tpcboxSame,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxSame(tpcbox, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpcbox_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxSame(tpcpatch, tpcbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpointcloud_tpcbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxSame(tpcpatch, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpointcloud_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = tpcboxSame,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tpcboxSame,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tpcboxSame,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Adjacent
 *****************************************************************************/

CREATE FUNCTION tpcboxAdjacent(tstzspan, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAdjacent(tpcpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = tpcboxAdjacent,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tpcboxAdjacent,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxAdjacent(tpcbox, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpcbox_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAdjacent(tpcpoint, tpcbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpointcloud_tpcbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAdjacent(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpointcloud_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = tpcboxAdjacent,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tpcboxAdjacent,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tpcboxAdjacent,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxAdjacent(tstzspan, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAdjacent(tpcpatch, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = tpcboxAdjacent,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tpcboxAdjacent,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION tpcboxAdjacent(tpcbox, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpcbox_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAdjacent(tpcpatch, tpcbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpointcloud_tpcbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAdjacent(tpcpatch, tpcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpointcloud_tpointcloud'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = tpcboxAdjacent,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tpcboxAdjacent,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tpcboxAdjacent,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/
