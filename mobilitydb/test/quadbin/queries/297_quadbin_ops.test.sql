-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

-- Static quadbin cell operations: resolution, hierarchy (parent/children/
-- sibling), grid disk (k-ring), point <-> cell, boundary/bounding box, area,
-- and the quadbin-unique tile/quadkey conversions.

-------------------------------------------------------------------------------
-- Resolution
-------------------------------------------------------------------------------

SELECT quadbin_get_resolution(quadbin '480fffffffffffff');  -- z0 world cell -> 0
SELECT quadbin_get_resolution(quadbin '48427fffffffffff');  -- tile(3,5,4) -> 4
SELECT quadbin_get_resolution(quadbin '48a6227affffffff');  -- res 10

-------------------------------------------------------------------------------
-- Hierarchy: parent / children / sibling
-------------------------------------------------------------------------------

-- Parent of a res-4 cell at res 0 is the z0 world cell
SELECT quadbin_cell_to_parent(quadbin '48427fffffffffff', 0) = quadbin '480fffffffffffff';
SELECT quadbin_get_resolution(quadbin_cell_to_parent(quadbin '48427fffffffffff', 2));

-- Children: exactly 4 per finer level
SELECT numValues(quadbin_cell_to_children(quadbin '480fffffffffffff', 1));
SELECT quadbin_cell_to_children(quadbin '480fffffffffffff', 1);

-- Round-trip: each child's parent is the origin cell
SELECT bool_and(quadbin_cell_to_parent(c, 0) = quadbin '480fffffffffffff')
  FROM unnest(quadbin_cell_to_children(quadbin '480fffffffffffff', 1)) AS c;

-- Sibling: moving right then left returns to the origin (same resolution)
SELECT quadbin_cell_sibling(quadbin_cell_sibling(quadbin '48427fffffffffff', 'right'), 'left')
  = quadbin '48427fffffffffff';
SELECT quadbin_get_resolution(quadbin_cell_sibling(quadbin '48427fffffffffff', 'up'));

-------------------------------------------------------------------------------
-- Grid disk (k-ring): (2k+1)^2 cells, origin included
-------------------------------------------------------------------------------

SELECT numValues(quadbin_grid_disk(quadbin '48a6227affffffff', 0));  -- 1
SELECT numValues(quadbin_grid_disk(quadbin '48a6227affffffff', 1));  -- 9
SELECT numValues(quadbin_grid_disk(quadbin '48a6227affffffff', 2));  -- 25

-------------------------------------------------------------------------------
-- Point <-> cell  (lon/lat, SRID 4326)
-------------------------------------------------------------------------------

-- Brussels (4.35, 50.85) at resolution 10
SELECT quadbin_point_to_cell(geometry 'SRID=4326;POINT(4.35 50.85)', 10)
  = quadbin '48a6227affffffff';

-- Resolution 0 maps any point to the world cell
SELECT quadbin_point_to_cell(geometry 'SRID=4326;POINT(4.35 50.85)', 0)
  = quadbin '480fffffffffffff';

-- Centroid of a cell is inside that cell -> mapping back recovers the cell
SELECT quadbin_point_to_cell(quadbin_cell_to_point(quadbin '48a6227affffffff'), 10)
  = quadbin '48a6227affffffff';

-- Centroid coordinates (SRID 4326)
SELECT round(ST_X(quadbin_cell_to_point(quadbin '48a6227affffffff'))::numeric, 6);
SELECT round(ST_Y(quadbin_cell_to_point(quadbin '48a6227affffffff'))::numeric, 6);
SELECT ST_SRID(quadbin_cell_to_point(quadbin '48a6227affffffff'));

-------------------------------------------------------------------------------
-- Boundary / bounding box
-------------------------------------------------------------------------------

SELECT ST_GeometryType(quadbin_cell_to_boundary(quadbin '48a6227affffffff'));
SELECT ST_GeometryType(quadbin_cell_to_bounding_box(quadbin '48a6227affffffff'));
SELECT ST_SRID(quadbin_cell_to_boundary(quadbin '48a6227affffffff'));

-- The centroid point lies inside the cell boundary polygon
SELECT ST_Contains(
  quadbin_cell_to_boundary(quadbin '48a6227affffffff'),
  quadbin_cell_to_point(quadbin '48a6227affffffff'));

-------------------------------------------------------------------------------
-- Area (square metres)
-------------------------------------------------------------------------------

-- The whole-world z0 cell area
SELECT round(quadbin_cell_area(quadbin '480fffffffffffff')::numeric, 1);
-- A finer cell has strictly smaller area than a coarser one
SELECT quadbin_cell_area(quadbin '48a6227affffffff')
  < quadbin_cell_area(quadbin '48427fffffffffff');

-------------------------------------------------------------------------------
-- Tile / quadkey conversion (quadbin-unique, no H3 analogue)
-------------------------------------------------------------------------------

-- Known CARTO vector: tile (0,0,0) is the z0 world cell 5192650370358181887
SELECT quadbin_tile_to_cell(0, 0, 0) = quadbin '480fffffffffffff';
SELECT (quadbin_tile_to_cell(0, 0, 0))::bigint;

-- tile (3,5,4) round trips
SELECT quadbin_tile_to_cell(3, 5, 4) = quadbin '48427fffffffffff';
SELECT quadbin_cell_to_tile(quadbin '48427fffffffffff');  -- {3,5,4}

-- quadkey: one base-4 digit per zoom level
SELECT quadbin_cell_to_quadkey(quadbin '480fffffffffffff');  -- '' (z0)
SELECT quadbin_cell_to_quadkey(quadbin '48427fffffffffff');  -- '0213'
SELECT quadbin_cell_to_quadkey(quadbin '48a6227affffffff');  -- '1202021322'

-------------------------------------------------------------------------------
