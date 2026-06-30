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

-- Kernel golden vectors: the MEOS quadbin kernel is transcribed bit-for-bit
-- from CARTO's reference quadbin-py implementation. Each assertion pins a
-- value computed by quadbin-py for a fixed (tile | lon/lat/res | cell) input,
-- so a kernel regression surfaces as a failed equality rather than a silent
-- drift. The expected integers / strings below are the quadbin-py outputs.

-------------------------------------------------------------------------------
-- tile_to_cell golden vector
--
-- quadbin-py: tile_to_cell((0, 0, 0)) == 5192650370358181887
-------------------------------------------------------------------------------

SELECT (quadbinTileToCell(0, 0, 0))::bigint = 5192650370358181887;
SELECT quadbinGetResolution(quadbinTileToCell(0, 0, 0)) = 0;

-- tile_to_cell((3, 5, 4)) -> the cell whose hex is 48427fffffffffff
SELECT quadbinTileToCell(3, 5, 4) = quadbin '48427fffffffffff';

-- tile_to_cell((524, 343, 10)) -> Brussels res-10 cell 48a6227affffffff
SELECT quadbinTileToCell(524, 343, 10) = quadbin '48a6227affffffff';

-------------------------------------------------------------------------------
-- cell_to_tile round trips (quadbin-py cell_to_tile inverse of tile_to_cell)
-------------------------------------------------------------------------------

SELECT quadbinCellToTile(quadbin '480fffffffffffff') = ARRAY[0, 0, 0];
SELECT quadbinCellToTile(quadbin '48427fffffffffff') = ARRAY[3, 5, 4];
SELECT quadbinCellToTile(quadbin '48a6227affffffff') = ARRAY[524, 343, 10];

-- Exhaustive small round trip: every (x, y) at zoom 3 survives tile->cell->tile
SELECT bool_and(quadbinCellToTile(quadbinTileToCell(x, y, 3)) = ARRAY[x, y, 3])
  FROM generate_series(0, 7) x, generate_series(0, 7) y;

-------------------------------------------------------------------------------
-- point_to_cell golden vector (quadbin-py point_to_cell)
--
-- quadbin-py: point_to_cell(4.35, 50.85, 10) == 48a6227affffffff (Brussels)
-- quadbin-py: point_to_cell(-73.985, 40.748, 18) == 49238451e2fbffff (NYC)
-------------------------------------------------------------------------------

SELECT geoToQuadbinCell(geometry 'SRID=4326;POINT(4.35 50.85)', 10)
  = quadbin '48a6227affffffff';
SELECT geoToQuadbinCell(geometry 'SRID=4326;POINT(-73.985 40.748)', 18)
  = quadbin '49238451e2fbffff';

-------------------------------------------------------------------------------
-- cell_to_point centroid golden vector (quadbin-py cell_to_point)
--
-- quadbin-py: cell_to_point(48a6227affffffff) centroid (4.394531, 50.847573)
-------------------------------------------------------------------------------

SELECT round(ST_X(quadbinCellToPoint(quadbin '48a6227affffffff'))::numeric, 6) = 4.394531;
SELECT round(ST_Y(quadbinCellToPoint(quadbin '48a6227affffffff'))::numeric, 6) = 50.847573;

-------------------------------------------------------------------------------
-- cell_to_bounding_box golden vector (quadbin-py cell_to_boundary bbox)
--
-- quadbin-py bbox(48a6227affffffff) = (4.218750, 50.736455, 4.570312, 50.958427)
-------------------------------------------------------------------------------

SELECT round(ST_XMin(quadbinCellToBoundingBox(quadbin '48a6227affffffff'))::numeric, 6) = 4.218750;
SELECT round(ST_YMin(quadbinCellToBoundingBox(quadbin '48a6227affffffff'))::numeric, 6) = 50.736455;
SELECT round(ST_XMax(quadbinCellToBoundingBox(quadbin '48a6227affffffff'))::numeric, 6) = 4.570312;
SELECT round(ST_YMax(quadbinCellToBoundingBox(quadbin '48a6227affffffff'))::numeric, 6) = 50.958427;

-------------------------------------------------------------------------------
-- cell_to_parent / cell_to_children golden vectors (quadbin-py hierarchy)
--
-- quadbin-py: cell_to_parent(48a6227affffffff, 5) == 485623ffffffffff
-- quadbin-py: cell_to_children(480fffffffffffff, 1) ==
--   {4813ffffffffffff, 4817ffffffffffff, 481bffffffffffff, 481fffffffffffff}
-------------------------------------------------------------------------------

SELECT quadbinCellToParent(quadbin '48a6227affffffff', 5) = quadbin '485623ffffffffff';
SELECT quadbinCellToChildren(quadbin '480fffffffffffff', 1)
  = quadbinset '{4813ffffffffffff, 4817ffffffffffff, 481bffffffffffff, 481fffffffffffff}';

-------------------------------------------------------------------------------
-- quadkey golden vectors (quadbin-py cell_to_tile -> quadkey)
--
-- z0 world cell -> '' ; tile(3,5,4) -> '0213' ; Brussels res10 -> '1202021322'
-------------------------------------------------------------------------------

SELECT quadbinCellToQuadkey(quadbin '480fffffffffffff') = '';
SELECT quadbinCellToQuadkey(quadbin '48427fffffffffff') = '0213';
SELECT quadbinCellToQuadkey(quadbin '48a6227affffffff') = '1202021322';

-------------------------------------------------------------------------------
