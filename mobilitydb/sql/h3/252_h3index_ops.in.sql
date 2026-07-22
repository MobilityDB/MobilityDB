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
 * @brief Static h3index cell operations.
 *
 * Non-temporal operations on the `h3index` base type: the static-geometry
 * → cell / cell-set constructors (`geoToH3Cell`, `geoToH3IndexSet`) and the
 * MobilityDB ports of the h3-pg SETOF-returning grid functions. h3-pg
 * returns rows; MobilityDB returns a single `h3indexset` (or `intset` for
 * icosahedron faces) — the finite collection companion type of `h3index`.
 * Every function here operates on STATIC `h3index` values; temporal variants
 * are deliberately parked until a `tset<T>` primitive exists.
 *
 * The geometry → cell constructors are C-wrapped in
 * `mobilitydb/src/h3/h3_geo.c`; the grid functions in
 * `mobilitydb/src/h3/h3index_sets.c`.
 */

/******************************************************************************
 * Static geometry → H3 cell (POINT only) / cell set (any geometry)
 ******************************************************************************/

CREATE FUNCTION geoToH3Cell(geometry, integer)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'Geo_point_to_h3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geoToH3IndexSet(geometry, integer)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Geo_to_h3indexset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Grid traversal
 ******************************************************************************/

CREATE FUNCTION h3GridDisk(h3index, integer)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'H3_grid_disk'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3GridRing(h3index, integer)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'H3_grid_ring'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3GridPathCells(h3index, h3index)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'H3_grid_path_cells'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Hierarchy
 ******************************************************************************/

CREATE FUNCTION h3CellToChildren(h3index, integer)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'H3_cell_to_children'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3CompactCells(h3indexset)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'H3_compact_cells'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3UncompactCells(h3indexset, integer)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'H3_uncompact_cells'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Edges and vertexes
 ******************************************************************************/

CREATE FUNCTION h3OriginToDirectedEdges(h3index)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'H3_origin_to_directed_edges'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3CellToVertexes(h3index)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'H3_cell_to_vertexes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Icosahedron faces
 ******************************************************************************/

CREATE FUNCTION h3GetIcosahedronFaces(h3index)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'H3_get_icosahedron_faces'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
