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

SELECT quadbinGetResolution(quadbin '480fffffffffffff');  -- z0 world cell -> 0
SELECT quadbinGetResolution(quadbin '48427fffffffffff');  -- tile(3,5,4) -> 4
SELECT quadbinGetResolution(quadbin '48a6227affffffff');  -- res 10

-------------------------------------------------------------------------------
-- Hierarchy: parent / children / sibling
-------------------------------------------------------------------------------

-- Parent of a res-4 cell at res 0 is the z0 world cell
SELECT quadbinCellToParent(quadbin '48427fffffffffff', 0) = quadbin '480fffffffffffff';
SELECT quadbinGetResolution(quadbinCellToParent(quadbin '48427fffffffffff', 2));

-- Children: exactly 4 per finer level
SELECT numValues(quadbinCellToChildren(quadbin '480fffffffffffff', 1));
SELECT quadbinCellToChildren(quadbin '480fffffffffffff', 1);

-- Round-trip: each child's parent is the origin cell
SELECT bool_and(quadbinCellToParent(c, 0) = quadbin '480fffffffffffff')
  FROM unnest(quadbinCellToChildren(quadbin '480fffffffffffff', 1)) AS c;

-- Sibling: moving right then left returns to the origin (same resolution)
SELECT quadbinCellSibling(quadbinCellSibling(quadbin '48427fffffffffff', 'right'), 'left')
  = quadbin '48427fffffffffff';
SELECT quadbinGetResolution(quadbinCellSibling(quadbin '48427fffffffffff', 'up'));

-------------------------------------------------------------------------------
-- Grid disk (k-ring): (2k+1)^2 cells, origin included
-------------------------------------------------------------------------------

SELECT numValues(quadbinGridDisk(quadbin '48a6227affffffff', 0));  -- 1
SELECT numValues(quadbinGridDisk(quadbin '48a6227affffffff', 1));  -- 9
SELECT numValues(quadbinGridDisk(quadbin '48a6227affffffff', 2));  -- 25

-------------------------------------------------------------------------------
-- Point <-> cell  (lon/lat, SRID 4326)
-------------------------------------------------------------------------------

-- Brussels (4.35, 50.85) at resolution 10
SELECT geoToQuadbinCell(geometry 'SRID=4326;POINT(4.35 50.85)', 10)
  = quadbin '48a6227affffffff';

-- Resolution 0 maps any point to the world cell
SELECT geoToQuadbinCell(geometry 'SRID=4326;POINT(4.35 50.85)', 0)
  = quadbin '480fffffffffffff';

-- Centroid of a cell is inside that cell -> mapping back recovers the cell
SELECT geoToQuadbinCell(quadbinCellToPoint(quadbin '48a6227affffffff'), 10)
  = quadbin '48a6227affffffff';

-- Centroid coordinates (SRID 4326)
SELECT round(ST_X(quadbinCellToPoint(quadbin '48a6227affffffff'))::numeric, 6);
SELECT round(ST_Y(quadbinCellToPoint(quadbin '48a6227affffffff'))::numeric, 6);
SELECT ST_SRID(quadbinCellToPoint(quadbin '48a6227affffffff'));

-------------------------------------------------------------------------------
-- Boundary / bounding box
-------------------------------------------------------------------------------

SELECT ST_GeometryType(quadbinCellToBoundary(quadbin '48a6227affffffff'));
SELECT ST_GeometryType(quadbinCellToBoundingBox(quadbin '48a6227affffffff'));
SELECT ST_SRID(quadbinCellToBoundary(quadbin '48a6227affffffff'));

-- The centroid point lies inside the cell boundary polygon
SELECT ST_Contains(
  quadbinCellToBoundary(quadbin '48a6227affffffff'),
  quadbinCellToPoint(quadbin '48a6227affffffff'));

-------------------------------------------------------------------------------
-- Area (square metres)
-------------------------------------------------------------------------------

-- The whole-world z0 cell area
SELECT round(quadbinCellArea(quadbin '480fffffffffffff')::numeric, 1);
-- A finer cell has strictly smaller area than a coarser one
SELECT quadbinCellArea(quadbin '48a6227affffffff')
  < quadbinCellArea(quadbin '48427fffffffffff');

-------------------------------------------------------------------------------
-- Tile / quadkey conversion (quadbin-unique, no H3 analogue)
-------------------------------------------------------------------------------

-- Known CARTO vector: tile (0,0,0) is the z0 world cell 5192650370358181887
SELECT quadbinTileToCell(0, 0, 0) = quadbin '480fffffffffffff';
SELECT (quadbinTileToCell(0, 0, 0))::bigint;

-- tile (3,5,4) round trips
SELECT quadbinTileToCell(3, 5, 4) = quadbin '48427fffffffffff';
SELECT quadbinCellToTile(quadbin '48427fffffffffff');  -- {3,5,4}

-- quadkey: one base-4 digit per zoom level
SELECT quadbinCellToQuadkey(quadbin '480fffffffffffff');  -- '' (z0)
SELECT quadbinCellToQuadkey(quadbin '48427fffffffffff');  -- '0213'
SELECT quadbinCellToQuadkey(quadbin '48a6227affffffff');  -- '1202021322'

-------------------------------------------------------------------------------
