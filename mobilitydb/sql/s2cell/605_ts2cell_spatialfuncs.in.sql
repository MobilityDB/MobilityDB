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
 * @brief Temporal `ts2cell` cell operations.
 *
 * The six operations every DGGS shares, lifted over a temporal value:
 * resolution, validity, the ancestor at a coarser level, the centre,
 * the boundary and the area. Each binds a wrapper in
 * `mobilitydb/src/s2cell/ts2cell_ops.c` that delegates to the generic
 * cell-index entry point, which reads the `s2_cellops` descriptor to
 * reach the S2 kernel — so the file adds no algorithm of its own.
 * `ts2CellToToken` is S2's own and has no H3 or QUADBIN analogue.
 *
 * An S2 cell is a region of the WGS84 sphere, so the centre is a
 * `tgeogpoint` and the boundary a `tgeography`, both in SRID 4326,
 * where the Web-Mercator `tquadbin` answers planar ones. The family
 * declares no SRID surface: its reference system is fixed by the S2
 * specification rather than carried by a value, as h3's is by its own
 * and quadbin's by the Bing tile scheme.
 */

CREATE FUNCTION getResolution(ts2cell)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Ts2cell_get_resolution'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION isValidCell(ts2cell)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ts2cell_is_valid_cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cellArea(ts2cell)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Ts2cell_cell_area'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cellToParent(ts2cell, integer)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Ts2cell_cell_to_parent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cellToPoint(ts2cell)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Ts2cell_cell_to_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cellToBoundary(ts2cell)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Ts2cell_cell_to_boundary'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- The centre is the cell's own point, so it is also the cast to a temporal
-- point, as `th3index` and `tquadbin` each declare one. S2 is defined on the
-- sphere, so the geodetic type is the one a cell answers in.
CREATE CAST (ts2cell AS tgeogpoint) WITH FUNCTION cellToPoint(ts2cell);

CREATE FUNCTION ts2CellToToken(ts2cell)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Ts2cell_cell_to_token'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Conversion from a temporal point
 *
 * `ts2cell(tgeogpoint, integer)` answers the temporal cell of a geodetic
 * trajectory, and `ts2cell(tgeompoint, integer)` the one of a trajectory in a
 * lon/lat reference system. Each segment is traversed cell by cell along the
 * path its point follows, the great circle of a geodetic point and the
 * straight line in longitude and latitude of a planar one, so the result holds
 * every cell the trajectory crosses.
 ******************************************************************************/

CREATE FUNCTION ts2cell(tgeogpoint, integer)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Tgeogpoint_to_ts2cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ts2cell(tgeompoint, integer)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Tgeompoint_to_ts2cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * s2cellset × ts2cell — ever-equal (sound cell-set prefilter)
 *
 * True when the temporal S2 cell ever equals a cell in the set. Paired with
 * geoToS2CellSet, this is the sound, conservative spatial prefilter for the
 * exact eIntersects(geography, ts2cell): the cell-granularity test never drops
 * a real intersection, and the exact predicate confirms the survivors.
 ******************************************************************************/

CREATE FUNCTION eEqual(s2cellset, ts2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_s2cellset_ts2cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eEqual(ts2cell, s2cellset)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT eEqual($2, $1) $$;

CREATE OPERATOR ?= (
  LEFTARG = s2cellset, RIGHTARG = ts2cell,
  PROCEDURE = eEqual,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = ts2cell, RIGHTARG = s2cellset,
  PROCEDURE = eEqual,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/
