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
 *****************************************************************************/

/**
 * @file
 * @brief Bounding-box operators for tpcpoint and tpcpatch — overlaps
 *   (&&), contains (\@>), contained (<\@), same (~=), and adjacent
 *   (-|-) — paired against tpcbox, tstzspan, and the temporal type
 *   itself. Mirrors the cbuffer / npoint topops surface.
 *
 * tstzspan-paired variants reuse the generic `Overlaps_tstzspan_temporal`
 * et al. PG functions (already exist for every temporal type); only
 * the SQL-level CREATE FUNCTION + CREATE OPERATOR declarations are
 * needed.
 *
 * tpcbox- and tpointcloud-paired variants use the type-specific PG
 * wrappers from `mobilitydb/src/pointcloud/tpc_boxops.c`.
 */

/******************************************************************************
 * Overlaps (&&)
 ******************************************************************************/

-- tpcpoint
CREATE FUNCTION temporal_overlaps(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpcbox_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpointcloud_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpointcloud_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (PROCEDURE = temporal_overlaps,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint, COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR && (PROCEDURE = temporal_overlaps,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan, COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR && (PROCEDURE = temporal_overlaps,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR && (PROCEDURE = temporal_overlaps,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR && (PROCEDURE = temporal_overlaps,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

-- tpcpatch
CREATE FUNCTION temporal_overlaps(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpcbox_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpointcloud_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpointcloud_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (PROCEDURE = temporal_overlaps,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch, COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR && (PROCEDURE = temporal_overlaps,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan, COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR && (PROCEDURE = temporal_overlaps,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR && (PROCEDURE = temporal_overlaps,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR && (PROCEDURE = temporal_overlaps,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

/******************************************************************************
 * Contains (@>)
 ******************************************************************************/

-- tpcpoint
CREATE FUNCTION temporal_contains(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpcbox_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpointcloud_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpointcloud_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (PROCEDURE = temporal_contains,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint, COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR @> (PROCEDURE = temporal_contains,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan, COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR @> (PROCEDURE = temporal_contains,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR @> (PROCEDURE = temporal_contains,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR @> (PROCEDURE = temporal_contains,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

-- tpcpatch
CREATE FUNCTION temporal_contains(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpcbox_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpointcloud_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpointcloud_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (PROCEDURE = temporal_contains,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch, COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR @> (PROCEDURE = temporal_contains,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan, COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR @> (PROCEDURE = temporal_contains,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR @> (PROCEDURE = temporal_contains,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR @> (PROCEDURE = temporal_contains,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

/******************************************************************************
 * Contained (<@)
 ******************************************************************************/

-- tpcpoint
CREATE FUNCTION temporal_contained(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpcbox_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpointcloud_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpointcloud_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (PROCEDURE = temporal_contained,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint, COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <@ (PROCEDURE = temporal_contained,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan, COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <@ (PROCEDURE = temporal_contained,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <@ (PROCEDURE = temporal_contained,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <@ (PROCEDURE = temporal_contained,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

-- tpcpatch
CREATE FUNCTION temporal_contained(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpcbox_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpointcloud_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpointcloud_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (PROCEDURE = temporal_contained,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch, COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <@ (PROCEDURE = temporal_contained,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan, COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <@ (PROCEDURE = temporal_contained,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <@ (PROCEDURE = temporal_contained,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <@ (PROCEDURE = temporal_contained,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

/******************************************************************************
 * Same (~=)
 ******************************************************************************/

-- tpcpoint
CREATE FUNCTION temporal_same(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpcbox_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpointcloud_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpointcloud_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (PROCEDURE = temporal_same,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint, COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR ~= (PROCEDURE = temporal_same,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan, COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR ~= (PROCEDURE = temporal_same,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR ~= (PROCEDURE = temporal_same,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR ~= (PROCEDURE = temporal_same,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

-- tpcpatch
CREATE FUNCTION temporal_same(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpcbox_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpointcloud_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpointcloud_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (PROCEDURE = temporal_same,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch, COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR ~= (PROCEDURE = temporal_same,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan, COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR ~= (PROCEDURE = temporal_same,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR ~= (PROCEDURE = temporal_same,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR ~= (PROCEDURE = temporal_same,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

/******************************************************************************
 * Adjacent (-|-)
 ******************************************************************************/

-- tpcpoint
CREATE FUNCTION temporal_adjacent(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpcbox_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpointcloud_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpointcloud_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (PROCEDURE = temporal_adjacent,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint, COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR -|- (PROCEDURE = temporal_adjacent,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan, COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR -|- (PROCEDURE = temporal_adjacent,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR -|- (PROCEDURE = temporal_adjacent,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR -|- (PROCEDURE = temporal_adjacent,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

-- tpcpatch
CREATE FUNCTION temporal_adjacent(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpcbox_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpointcloud_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpointcloud_tpointcloud'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (PROCEDURE = temporal_adjacent,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch, COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR -|- (PROCEDURE = temporal_adjacent,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan, COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR -|- (PROCEDURE = temporal_adjacent,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR -|- (PROCEDURE = temporal_adjacent,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR -|- (PROCEDURE = temporal_adjacent,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

/*****************************************************************************/
