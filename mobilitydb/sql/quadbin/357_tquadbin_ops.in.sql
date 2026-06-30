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
 * @brief Accessors, lifted cell operations, and convenience casts for
 * `tquadbin`.
 *
 * Value accessors (`startValue`, `endValue`, `valueN`, `getValues`,
 * `valueAtTimestamp`) route to the generic `Temporal_*` C symbols —
 * the same pattern tbigint and th3index use — and return the quadbin
 * basetype family (`quadbin` for scalar accessors, `quadbinset` for
 * `getValues`).
 *
 * The lifted cell operations dispatch through the generic
 * `tcellindex_*` DGGS entry points (resolution, validity, parent,
 * centroid point, boundary, area) shared by every cell-index temporal
 * type, plus the quadbin-unique `tquadbin_cell_to_quadkey`. Each lifts
 * its static kernel over the time axis, preserving the input's
 * instants.
 */

/******************************************************************************
 * Value accessors
 ******************************************************************************/

CREATE FUNCTION startValue(tquadbin)
  RETURNS quadbin
  AS 'MODULE_PATHNAME', 'Temporal_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(tquadbin)
  RETURNS quadbin
  AS 'MODULE_PATHNAME', 'Temporal_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(tquadbin, integer)
  RETURNS quadbin
  AS 'MODULE_PATHNAME', 'Temporal_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getValues(tquadbin)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Temporal_valueset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueAtTimestamp(tquadbin, timestamptz)
  RETURNS quadbin
  AS 'MODULE_PATHNAME', 'Temporal_value_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Resolution + validity (unary_scalar lifts)
 ******************************************************************************/

CREATE FUNCTION tquadbinGetResolution(tquadbin)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Tquadbin_get_resolution'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION isValidCell(tquadbin)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tquadbin_is_valid_cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Hierarchy
 *
 * `tquadbinCellToParent` lifts via `lift_with_const` — the integer
 * resolution is constant across the time axis.
 ******************************************************************************/

CREATE FUNCTION tquadbinCellToParent(tquadbin, integer)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Tquadbin_cell_to_parent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Centroid point / boundary (Web-Mercator lon/lat, SRID 4326)
 *
 * `tquadbinCellToPoint` emits the per-instant cell centroid as a
 * `tgeompoint`; `tquadbinCellToBoundary` emits the per-instant
 * square polygon as a `tgeometry`.
 ******************************************************************************/

CREATE FUNCTION tquadbinCellToPoint(tquadbin)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Tquadbin_cell_to_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tquadbinCellToBoundary(tquadbin)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Tquadbin_cell_to_boundary'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Area
 *
 * `tquadbinCellArea` lifts the per-cell area (square metres) over the
 * time axis.
 ******************************************************************************/

CREATE FUNCTION tquadbinCellArea(tquadbin)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tquadbin_cell_area'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Quadkey conversion (quadbin-unique, no H3 analogue)
 *
 * `tquadbinCellToQuadkey` lifts the base-4 slippy-tile quadkey
 * string per instant, yielding a `ttext`.
 ******************************************************************************/

CREATE FUNCTION tquadbinCellToQuadkey(tquadbin)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tquadbin_cell_to_quadkey'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Convenience cast: tquadbin :: tgeompoint
 *
 * Reuses the lifted `tquadbinCellToPoint` centroid conversion.
 * EXPLICIT (not IMPLICIT nor ASSIGNMENT): typing a cell trajectory as
 * a point trajectory should not happen by accident. The
 * `tquadbin :: tbigint` and `tbigint :: tquadbin` casts are declared
 * in `352_tquadbin.in.sql` as ASSIGNMENT and are not re-declared here.
 ******************************************************************************/

CREATE CAST (tquadbin AS tgeompoint)
  WITH FUNCTION tquadbinCellToPoint(tquadbin);

/******************************************************************************/
