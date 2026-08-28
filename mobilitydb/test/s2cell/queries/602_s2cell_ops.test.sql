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

-- Static s2cell cell operations: resolution, the cube face, hierarchy, the
-- Hilbert range, the centre and boundary geographies, area and edge length,
-- and the token.
--
-- Every cell here is derived by geoToS2Cell from a point, so the answers
-- cross-check against the level asked for rather than against a literal.

-------------------------------------------------------------------------------
-- Resolution and face
-------------------------------------------------------------------------------

SELECT getResolution(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10));
SELECT getResolution(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 20));
SELECT s2GetFace(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10));
SELECT s2GetFace(geoToS2Cell(geography 'SRID=4326;Point(-122.4 37.8)', 10));

-------------------------------------------------------------------------------
-- Hierarchy
--
-- A parent contains every cell it is the parent of, and the level of the
-- deepest cell containing a cell and its own parent is the parent's level.
-------------------------------------------------------------------------------

SELECT getResolution(cellToParent(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 15), 8));
SELECT s2CellContains(cellToParent(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 15), 8),
  geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 15));
SELECT s2CellContains(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 15),
  cellToParent(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 15), 8));
SELECT s2CommonAncestorLevel(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 15),
  cellToParent(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 15), 8));
SELECT s2CommonAncestorLevel(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10),
  geoToS2Cell(geography 'SRID=4326;Point(-122.4 37.8)', 10));
SELECT getResolution(s2CellToChild(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 8), 9, 0));
SELECT s2CellContains(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 8),
  s2CellToChild(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 8), 9, 2));
-- A child is exactly one level finer than the cell it is a child of.
SELECT s2CellToChild(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 8), 10, 0);
SELECT numValues(s2CellToChildren(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 8), 9));
SELECT numValues(s2CellToChildren(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 8), 10));

-------------------------------------------------------------------------------
-- The Hilbert range
--
-- A cell spans a contiguous run of leaf cells, so its own identifier lies
-- between the two bounds and a descendant does too.
-------------------------------------------------------------------------------

SELECT s2RangeMin(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10))
  <= geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10);
SELECT s2RangeMax(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10))
  >= geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10);
SELECT geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 20)
  BETWEEN s2RangeMin(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10))
  AND s2RangeMax(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10));
SELECT getResolution(s2RangeMin(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10)));

-------------------------------------------------------------------------------
-- Edge neighbours
-------------------------------------------------------------------------------

SELECT numValues(s2EdgeNeighbors(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10)));
SELECT geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10)
  <@ s2EdgeNeighbors(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10));

-------------------------------------------------------------------------------
-- Centre and boundary
--
-- The centre lies in the cell it comes from, and the boundary is a polygon of
-- four corners closed back on the first.
-------------------------------------------------------------------------------

SELECT geoToS2Cell(cellToPoint(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10)), 10)
  = geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10);
SELECT ST_SRID(cellToPoint(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10))::geometry);
SELECT ST_GeometryType(cellToBoundary(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10))::geometry);
SELECT ST_NPoints(cellToBoundary(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10))::geometry);
SELECT ST_SRID(cellToBoundary(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10))::geometry);

-------------------------------------------------------------------------------
-- Area and edge length
--
-- The whole sphere is the six face cells, so a face cell covers a sixth of
-- the WGS84 area of 510 065 621 sq km, and a cell four levels finer covers
-- about a 256th of its parent.
-------------------------------------------------------------------------------

SELECT round(cellArea(cellToParent(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10), 0)) / 1e12, 3);
SELECT round(cellArea(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10))
  / cellArea(cellToParent(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10), 6)), 4);
SELECT round(s2EdgeLength(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10), 0) / 1000, 3);
SELECT round(s2EdgeLength(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10), 2) / 1000, 3);

-------------------------------------------------------------------------------
-- The token
--
-- The token is the identifier in hexadecimal with the trailing zeros removed,
-- and it round-trips.
-------------------------------------------------------------------------------

SELECT s2CellToToken(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10));
SELECT s2CellToToken(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 20));
SELECT s2TokenToCell(s2CellToToken(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10)))
  = geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10);

-------------------------------------------------------------------------------
