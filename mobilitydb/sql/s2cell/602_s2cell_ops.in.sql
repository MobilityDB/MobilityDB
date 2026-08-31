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
 * @brief Static `s2cell` cell operations.
 *
 * The subset of the DGGS surface every family shares — resolution,
 * hierarchy, cell ↔ point, boundary and area — under the names the
 * `DggsCellOps` descriptor in `meos/include/temporal/tcellindex.h`
 * fixes, together with the operations that are S2's own: the cube
 * face a cell sits on, the Hilbert range a cell spans, the level of
 * the deepest cell containing two cells, and the token.
 *
 * A cell is a region of the WGS84 sphere, so its centre and its
 * boundary are geographies in SRID 4326 and its area is in square
 * metres. There is no k-ring: the Hilbert curve gives four edge
 * neighbours and no ring of a given radius.
 *
 * C wrappers in `mobilitydb/src/s2cell/s2cell_ops.c`; first-party
 * kernel in `meos/src/s2cell/s2cell.c`.
 */

CREATE FUNCTION getResolution(s2cell)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'S2cell_get_resolution'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2GetFace(s2cell)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'S2cell_get_face'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cellArea(s2cell)
  RETURNS float
  AS 'MODULE_PATHNAME', 'S2cell_cell_area'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2EdgeLength(s2cell, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'S2cell_edge_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cellToParent(s2cell, integer)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'S2cell_cell_to_parent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2CellToChild(s2cell, integer, integer)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'S2cell_cell_to_child'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cellToChildren(s2cell, integer)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'S2cell_cell_to_children'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2CellContains(s2cell, s2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'S2cell_cell_contains'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2CommonAncestorLevel(s2cell, s2cell)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'S2cell_common_ancestor_level'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2RangeMin(s2cell)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'S2cell_range_min'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2RangeMax(s2cell)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'S2cell_range_max'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2EdgeNeighbors(s2cell)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'S2cell_edge_neighbors'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geoToS2Cell(geography, integer)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'S2cell_point_to_cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cellToPoint(s2cell)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'S2cell_cell_to_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cellToBoundary(s2cell)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'S2cell_cell_to_boundary'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2CellToToken(s2cell)
  RETURNS text
  AS 'MODULE_PATHNAME', 'S2cell_cell_to_token'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2TokenToCell(text)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'S2cell_token_to_cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
