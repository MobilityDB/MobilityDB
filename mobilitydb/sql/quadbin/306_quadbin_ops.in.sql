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
 * @brief Static `quadbin` cell operations.
 *
 * The square-grid subset of the DGGS surface: resolution, hierarchy
 * (parent / children / sibling), point ↔ cell, boundary, bounding
 * box, area, and the quadbin-unique slippy-tile (x / y / z) and
 * quadkey conversions. The hexagon-only families (directed edges,
 * vertices, pentagon / base-cell / class-III inspection, local-IJ
 * traversal, great-circle metrics) have no quadbin analogue and are
 * absent.
 *
 * The cell coordinates are CARTO Web-Mercator tiles; the emitted
 * geometry is lon/lat in SRID 4326. C wrappers in
 * `mobilitydb/src/quadbin/quadbin_ops.c`; first-party kernel in
 * `meos/src/quadbin/quadbin.c`.
 */

/******************************************************************************
 * Resolution
 ******************************************************************************/

CREATE FUNCTION quadbin_get_resolution(quadbin)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Quadbin_get_resolution'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Hierarchy
 *
 * `quadbin_cell_to_parent` drops to the requested coarser resolution;
 * `quadbin_cell_to_children` returns the four child cells at the
 * requested finer resolution as a `quadbinset`. `quadbin_cell_sibling`
 * returns the neighbouring cell at the same resolution in the given
 * direction (`up` / `down` / `left` / `right`).
 ******************************************************************************/

CREATE FUNCTION quadbin_cell_to_parent(quadbin, integer)
  RETURNS quadbin
  AS 'MODULE_PATHNAME', 'Quadbin_cell_to_parent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbin_cell_to_children(quadbin, integer)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Quadbin_cell_to_children'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbin_cell_sibling(quadbin, text)
  RETURNS quadbin
  AS 'MODULE_PATHNAME', 'Quadbin_cell_sibling'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Grid traversal — k-ring
 *
 * `quadbin_grid_disk` returns every cell within grid distance k of the
 * origin (the origin plus its k-step square neighbourhood) as a
 * `quadbinset`.
 ******************************************************************************/

CREATE FUNCTION quadbin_grid_disk(quadbin, integer)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Quadbin_grid_disk'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Point ↔ cell
 *
 * `quadbin_point_to_cell` maps a lon/lat point (SRID 4326) at the
 * given resolution to its quadbin cell; `quadbin_cell_to_point`
 * returns the cell centroid as a lon/lat point.
 ******************************************************************************/

CREATE FUNCTION quadbin_point_to_cell(geometry, integer)
  RETURNS quadbin
  AS 'MODULE_PATHNAME', 'Quadbin_point_to_cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbin_cell_to_point(quadbin)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Quadbin_cell_to_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Boundary / bounding box
 *
 * `quadbin_cell_to_boundary` returns the cell as a polygon (the four
 * corners of the square tile); `quadbin_cell_to_bounding_box` returns
 * the axis-aligned envelope geometry. Both emit lon/lat in SRID 4326.
 ******************************************************************************/

CREATE FUNCTION quadbin_cell_to_boundary(quadbin)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Quadbin_cell_to_boundary'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbin_cell_to_bounding_box(quadbin)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Quadbin_cell_to_bounding_box'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Area
 *
 * `quadbin_cell_area` returns the cell area in square metres.
 ******************************************************************************/

CREATE FUNCTION quadbin_cell_area(quadbin)
  RETURNS double precision
  AS 'MODULE_PATHNAME', 'Quadbin_cell_area'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Tile / quadkey conversion (quadbin-unique, no H3 analogue)
 *
 * `quadbin_tile_to_cell` builds a cell from slippy-map (x, y, z) tile
 * coordinates; `quadbin_cell_to_tile` decomposes a cell back into its
 * (x, y, z) tuple, returned as an integer array `{x, y, z}`.
 * `quadbin_cell_to_quadkey` emits the base-4 slippy-tile quadkey
 * string (one digit per zoom level, MSB-first; the z0 world cell maps
 * to the empty string).
 ******************************************************************************/

CREATE FUNCTION quadbin_tile_to_cell(integer, integer, integer)
  RETURNS quadbin
  AS 'MODULE_PATHNAME', 'Quadbin_tile_to_cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbin_cell_to_tile(quadbin)
  RETURNS integer[]
  AS 'MODULE_PATHNAME', 'Quadbin_cell_to_tile'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbin_cell_to_quadkey(quadbin)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Quadbin_cell_to_quadkey'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
