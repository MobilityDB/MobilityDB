-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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

-- A geography other than a point, an empty point and a level outside 0 to 30
-- are refused
/* Errors */
SELECT geoToS2Cell(geography 'SRID=4326;LINESTRING(4.35 50.85, 4.36 50.86)', 10);
SELECT geoToS2Cell(geography 'SRID=4326;POINT EMPTY', 10);
SELECT geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 31);
SELECT geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', -1);

-------------------------------------------------------------------------------
-- Geography -> cell set
-------------------------------------------------------------------------------

-- A point gives the set of its one cell
SELECT numValues(geoToS2CellSet(geography 'SRID=4326;Point(4.35 50.85)', 10)),
  startValue(geoToS2CellSet(geography 'SRID=4326;Point(4.35 50.85)', 10)) =
  geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 10);

-- The same point twice gives one cell
SELECT numValues(geoToS2CellSet(
  geography 'SRID=4326;MULTIPOINT((4.35 50.85), (4.35 50.85))', 10));

-- THE COVER IS EXACTLY THE CELLS THE GEOGRAPHY MEETS. Each geography is read
-- against the cells of level 13 inside the cell of level 7 holding it, those
-- whose geodesic boundary it intersects, and the query states how many cells
-- each side holds and how many each holds that the other does not. The line
-- crosses cells corner-wise, the hole of the first polygon is wider than a
-- cell, and the rows of the concave polygon leave cells outside it between its
-- two arms
WITH parent(cell) AS (VALUES
  (geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 7))),
geogs(name, geog) AS (VALUES
  ('concave', geography 'SRID=4326;POLYGON((4.30 50.80, 4.45 50.80,
    4.45 50.90, 4.40 50.90, 4.40 50.83, 4.35 50.83, 4.35 50.90, 4.30 50.90,
    4.30 50.80))'),
  ('holed', geography 'SRID=4326;POLYGON((4.30 50.80, 4.45 50.80, 4.45 50.90,
    4.30 50.90, 4.30 50.80), (4.34 50.83, 4.41 50.83, 4.41 50.87, 4.34 50.87,
    4.34 50.83))'),
  ('line', geography 'SRID=4326;LINESTRING(4.30 50.80, 4.45 50.90)')),
cover AS (
  SELECT name, unnest(geoToS2CellSet(geog, 13)) AS cell FROM geogs),
oracle AS (
  SELECT g.name, c.cell
  FROM geogs g, parent p, unnest(cellToChildren(p.cell, 13)) AS c(cell)
  WHERE ST_Intersects(cellToBoundary(c.cell), g.geog))
SELECT g.name,
  ST_Covers(cellToBoundary(p.cell), g.geog) AS inside_parent,
  (SELECT count(*) FROM cover c WHERE c.name = g.name) AS cover,
  (SELECT count(*) FROM oracle o WHERE o.name = g.name) AS oracle,
  (SELECT count(*) FROM (SELECT cell FROM cover WHERE name = g.name
     EXCEPT SELECT cell FROM oracle WHERE name = g.name) AS x) AS cover_only,
  (SELECT count(*) FROM (SELECT cell FROM oracle WHERE name = g.name
     EXCEPT SELECT cell FROM cover WHERE name = g.name) AS y) AS oracle_only
FROM geogs g, parent p
ORDER BY g.name;

-- A collection gives the union of the covers of its components
SELECT geoToS2CellSet(geography 'SRID=4326;GEOMETRYCOLLECTION(
    POINT(4.35 50.85), LINESTRING(4.40 50.88, 4.42 50.90),
    POLYGON((4.30 50.80, 4.32 50.80, 4.32 50.82, 4.30 50.82, 4.30 50.80)))',
    12) =
  setUnion(setUnion(
    geoToS2CellSet(geography 'SRID=4326;POINT(4.35 50.85)', 12),
    geoToS2CellSet(geography 'SRID=4326;LINESTRING(4.40 50.88, 4.42 50.90)',
      12)),
    geoToS2CellSet(geography 'SRID=4326;POLYGON((4.30 50.80, 4.32 50.80,
      4.32 50.82, 4.30 50.82, 4.30 50.80))', 12));

-- A polygon holding a pole, where the faces of the cube meet: the cover holds
-- the cells around the pole that no ring passes through, the cell of the pole
-- among them, and every cell of the cover meets the polygon
WITH p(poly, ring) AS (VALUES (
  geography 'SRID=4326;POLYGON((0 88, 120 88, -120 88, 0 88))',
  geography 'SRID=4326;LINESTRING(0 88, 120 88, -120 88, 0 88)'))
SELECT numValues(geoToS2CellSet(poly, 10)) > numValues(geoToS2CellSet(ring, 10)),
  geoToS2Cell(geography 'SRID=4326;Point(0 90)', 10) <@
    geoToS2CellSet(poly, 10),
  (SELECT bool_and(ST_Intersects(cellToBoundary(c), poly))
     FROM unnest(geoToS2CellSet(poly, 10)) AS c)
FROM p;

-- An empty geography holds no cell
SELECT geoToS2CellSet(geography 'SRID=4326;POINT EMPTY', 10) IS NULL;

-- A level outside 0 to 30 and a cover of more cells than a set is built from
/* Errors */
SELECT geoToS2CellSet(geography 'SRID=4326;Point(4.35 50.85)', 31);
SELECT geoToS2CellSet(geography 'SRID=4326;Point(4.35 50.85)', -1);
SELECT geoToS2CellSet(geography 'SRID=4326;POLYGON((4.30 50.80, 4.45 50.80,
  4.45 50.90, 4.30 50.90, 4.30 50.80))', 30);

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
SELECT numValues(cellToChildren(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 8), 9));
SELECT numValues(cellToChildren(geoToS2Cell(geography 'SRID=4326;Point(4.35 50.85)', 8), 10));

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
-- compactCells / uncompactCells
-------------------------------------------------------------------------------

-- An S2 cell is exactly the union of its four children, so the sixteen
-- grandchildren of a cell compact to the cell and uncompact back to themselves,
-- while the six faces, which have no parent, stay six
SELECT numValues(compactCells(cellToChildren(s2cell '47c3c', 9))),
  startValue(compactCells(cellToChildren(s2cell '47c3c', 9))) = s2cell '47c3c',
  uncompactCells(compactCells(cellToChildren(s2cell '47c3c', 9)), 9) =
    cellToChildren(s2cell '47c3c', 9),
  (SELECT numValues(compactCells(setUnion(geoToS2Cell(g, 0))))
   FROM unnest(ARRAY[geography 'Point(0 0)', 'Point(90 0)', 'Point(0 90)',
     'Point(180 0)', 'Point(-90 0)', 'Point(0 -90)']) AS g) AS faces;

-- Without one grandchild, three children of the cell and the three siblings of
-- the missing grandchild remain. A cell covered by a coarser cell of the set is
-- dropped, and the region a set states is kept: the compacted set and the set
-- uncompact to the same cells
WITH g(c) AS (
  SELECT unnest(cellToChildren(s2cell '47c3c', 9))),
part(cells) AS (
  SELECT setUnion(c) FROM g
  WHERE c <> startValue(cellToChildren(s2cell '47c3c', 9))),
mixed(cells) AS (
  SELECT setUnion(c) FROM (
    SELECT c FROM g
    WHERE c <> startValue(cellToChildren(s2cell '47c3c', 9))
    UNION ALL
    SELECT unnest(cellToChildren(s2cell '47c3c', 8))) AS u(c))
SELECT numValues(compactCells(part.cells)) AS without_one,
  numValues(compactCells(mixed.cells)) = 1 AND
    startValue(compactCells(mixed.cells)) = s2cell '47c3c' AS with_children,
  uncompactCells(compactCells(part.cells), 9) = uncompactCells(part.cells, 9)
    AS same_region
FROM part, mixed;

-- The compacted cover of a polygon states the region of the cover with fewer
-- cells: it uncompacts back to the cover and holds no four children of one
-- parent
WITH s(cells) AS (VALUES (geoToS2CellSet(geography 'SRID=4326;POLYGON((4.30 50.80,
  4.45 50.80, 4.45 50.95, 4.30 50.95, 4.30 50.80))', 14)))
SELECT numValues(cells) AS cover, numValues(compactCells(cells)) AS compacted,
  uncompactCells(compactCells(cells), 14) = cells AS same_region,
  (SELECT count(*) FROM (
     SELECT cellToParent(c, getResolution(c) - 1)
     FROM unnest(compactCells(cells)) AS c
     GROUP BY 1 HAVING count(*) = 4) AS q) AS groups_of_four
FROM s;

/* Errors */
-- A cell finer than the level, a result of more than 4194304 cells, and a level
-- outside 0 to 30
SELECT uncompactCells(cellToChildren(s2cell '47c3c', 9), 8);
SELECT uncompactCells(compactCells(cellToChildren(s2cell '47c3c', 8)), 19);
SELECT uncompactCells(cellToChildren(s2cell '47c3c', 9), 31);

-------------------------------------------------------------------------------
