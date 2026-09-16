-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
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

SELECT getResolution(quadbin '480fffffffffffff');  -- z0 world cell -> 0
SELECT getResolution(quadbin '48427fffffffffff');  -- tile(3,5,4) -> 4
SELECT getResolution(quadbin '48a6227affffffff');  -- res 10

-------------------------------------------------------------------------------
-- Hierarchy: parent / children / sibling
-------------------------------------------------------------------------------

-- Parent of a res-4 cell at res 0 is the z0 world cell
SELECT cellToParent(quadbin '48427fffffffffff', 0) = quadbin '480fffffffffffff';
SELECT getResolution(cellToParent(quadbin '48427fffffffffff', 2));

-- Children: exactly 4 per finer level
SELECT numValues(cellToChildren(quadbin '480fffffffffffff', 1));
SELECT cellToChildren(quadbin '480fffffffffffff', 1);

-- Round-trip: each child's parent is the origin cell
SELECT bool_and(cellToParent(c, 0) = quadbin '480fffffffffffff')
  FROM unnest(cellToChildren(quadbin '480fffffffffffff', 1)) AS c;

-- Sibling: moving right then left returns to the origin (same resolution)
SELECT quadbinCellSibling(quadbinCellSibling(quadbin '48427fffffffffff', 'right'), 'left')
  = quadbin '48427fffffffffff';
SELECT getResolution(quadbinCellSibling(quadbin '48427fffffffffff', 'up'));

-------------------------------------------------------------------------------
-- Grid disk (k-ring): (2k+1)^2 cells, origin included
-------------------------------------------------------------------------------

SELECT numValues(gridDisk(quadbin '48a6227affffffff', 0));  -- 1
SELECT numValues(gridDisk(quadbin '48a6227affffffff', 1));  -- 9
SELECT numValues(gridDisk(quadbin '48a6227affffffff', 2));  -- 25

-- The disk of a set is the union of the disks of its cells. Widening the disk
-- of radius 1 by one step gives the disk of radius 2 around the same origin,
-- and a step count of 0 gives the set itself
WITH s(cells) AS (VALUES (gridDisk(quadbin '48a6227affffffff', 1)))
SELECT numValues(gridDisk(cells, 1)),
  gridDisk(cells, 1) = (SELECT setUnion(d)
    FROM unnest(cells) AS c, unnest(gridDisk(c, 1)) AS d),
  gridDisk(cells, 1) = gridDisk(quadbin '48a6227affffffff', 2),
  gridDisk(cells, 0) = cells
FROM s;

/* Errors */
SELECT gridDisk(gridDisk(quadbin '48a6227affffffff', 1), -1);

-------------------------------------------------------------------------------
-- compactCells / uncompactCells
-------------------------------------------------------------------------------

-- A tile is exactly the union of its four children, so the sixteen
-- grandchildren of a tile compact to the tile and uncompact back to themselves
SELECT numValues(compactCells(cellToChildren(quadbin '48a6227affffffff', 12))),
  startValue(compactCells(cellToChildren(quadbin '48a6227affffffff', 12))) =
    quadbin '48a6227affffffff',
  uncompactCells(compactCells(cellToChildren(quadbin '48a6227affffffff', 12)), 12) =
    cellToChildren(quadbin '48a6227affffffff', 12);

-- Without one grandchild, three children of the tile and the three siblings of
-- the missing grandchild remain. A cell covered by a coarser cell of the set is
-- dropped, so adding the children of the tile, of a coarser resolution, gives
-- the tile again, and the region a set states is kept: the compacted set and
-- the set uncompact to the same cells
WITH g(c) AS (
  SELECT unnest(cellToChildren(quadbin '48a6227affffffff', 12))),
part(cells) AS (
  SELECT setUnion(c) FROM g
  WHERE c <> startValue(cellToChildren(quadbin '48a6227affffffff', 12))),
mixed(cells) AS (
  SELECT setUnion(c) FROM (
    SELECT c FROM g
    WHERE c <> startValue(cellToChildren(quadbin '48a6227affffffff', 12))
    UNION ALL
    SELECT unnest(cellToChildren(quadbin '48a6227affffffff', 11))) AS u(c))
SELECT numValues(compactCells(part.cells)) AS without_one,
  compactCells(mixed.cells) = quadbinset '{48a6227affffffff}' AS with_children,
  uncompactCells(compactCells(part.cells), 12) = uncompactCells(part.cells, 12)
    AS same_region
FROM part, mixed;

-- The compacted cover of a polygon states the region of the cover with fewer
-- cells: it uncompacts back to the cover and holds no four children of one
-- parent
WITH s(cells) AS (VALUES (geoToQuadbinSet(geometry 'SRID=4326;POLYGON((4.30 50.80,
  4.45 50.80, 4.45 50.95, 4.30 50.95, 4.30 50.80))', 14)))
SELECT numValues(cells) AS cover, numValues(compactCells(cells)) AS compacted,
  uncompactCells(compactCells(cells), 14) = cells AS same_region,
  (SELECT count(*) FROM (
     SELECT cellToParent(c, getResolution(c) - 1)
     FROM unnest(compactCells(cells)) AS c
     GROUP BY 1 HAVING count(*) = 4) AS q) AS groups_of_four
FROM s;

/* Errors */
-- A cell finer than the resolution, a result of more than 4194304 cells, and a
-- resolution outside 0 to 26
SELECT uncompactCells(cellToChildren(quadbin '48a6227affffffff', 12), 11);
SELECT uncompactCells(quadbinset '{480fffffffffffff}', 12);
SELECT uncompactCells(cellToChildren(quadbin '48a6227affffffff', 12), 27);

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
SELECT geoToQuadbinCell(cellToPoint(quadbin '48a6227affffffff'), 10)
  = quadbin '48a6227affffffff';

-- A geometry other than a point, an empty point and a resolution outside 0 to
-- 26 are refused
/* Errors */
SELECT geoToQuadbinCell(geometry 'SRID=4326;LINESTRING(4.35 50.85, 4.36 50.86)', 10);
SELECT geoToQuadbinCell(geometry 'SRID=4326;POINT EMPTY', 10);
SELECT geoToQuadbinCell(geometry 'SRID=4326;POINT(4.35 50.85)', 27);
SELECT geoToQuadbinCell(geometry 'SRID=4326;POINT(4.35 50.85)', -1);

-- Centroid coordinates (SRID 4326)
SELECT round(ST_X(cellToPoint(quadbin '48a6227affffffff'))::numeric, 6);
SELECT round(ST_Y(cellToPoint(quadbin '48a6227affffffff'))::numeric, 6);
SELECT ST_SRID(cellToPoint(quadbin '48a6227affffffff'));

-------------------------------------------------------------------------------
-- Geometry -> cell set  (lon/lat, SRID 4326)
-------------------------------------------------------------------------------

-- A point gives the set of its one cell
SELECT numValues(geoToQuadbinSet(geometry 'SRID=4326;POINT(4.35 50.85)', 10)),
  startValue(geoToQuadbinSet(geometry 'SRID=4326;POINT(4.35 50.85)', 10)) =
  geoToQuadbinCell(geometry 'SRID=4326;POINT(4.35 50.85)', 10);

-- The same point twice gives one cell
SELECT numValues(geoToQuadbinSet(
  geometry 'SRID=4326;MULTIPOINT((4.35 50.85), (4.35 50.85))', 10));

-- THE COVER IS EXACTLY THE TILES THE GEOMETRY MEETS. Each geometry is read
-- against the tiles of resolution 15 inside the tile of resolution 8 holding
-- it, those whose boundary it intersects, and the query states how many cells
-- each side holds and how many each holds that the other does not. The line
-- crosses tiles corner-wise, the hole of the first polygon is wider than a
-- tile, and the rows of the concave polygon leave tiles outside it between its
-- two arms
WITH parent(cell) AS (VALUES
  (geoToQuadbinCell(geometry 'SRID=4326;POINT(4.35 50.85)', 8))),
geoms(name, geom) AS (VALUES
  ('concave', geometry 'SRID=4326;POLYGON((4.30 50.80, 4.45 50.80,
    4.45 50.90, 4.40 50.90, 4.40 50.83, 4.35 50.83, 4.35 50.90, 4.30 50.90,
    4.30 50.80))'),
  ('holed', geometry 'SRID=4326;POLYGON((4.30 50.80, 4.45 50.80, 4.45 50.90,
    4.30 50.90, 4.30 50.80), (4.34 50.83, 4.41 50.83, 4.41 50.87, 4.34 50.87,
    4.34 50.83))'),
  ('line', geometry 'SRID=4326;LINESTRING(4.30 50.80, 4.45 50.90)')),
cover AS (
  SELECT name, unnest(geoToQuadbinSet(geom, 15)) AS cell FROM geoms),
oracle AS (
  SELECT g.name, c.cell
  FROM geoms g, parent p, unnest(cellToChildren(p.cell, 15)) AS c(cell)
  WHERE ST_Intersects(cellToBoundary(c.cell), g.geom))
SELECT g.name,
  ST_Covers(cellToBoundary(p.cell), g.geom) AS inside_parent,
  (SELECT count(*) FROM cover c WHERE c.name = g.name) AS cover,
  (SELECT count(*) FROM oracle o WHERE o.name = g.name) AS oracle,
  (SELECT count(*) FROM (SELECT cell FROM cover WHERE name = g.name
     EXCEPT SELECT cell FROM oracle WHERE name = g.name) AS x) AS cover_only,
  (SELECT count(*) FROM (SELECT cell FROM oracle WHERE name = g.name
     EXCEPT SELECT cell FROM cover WHERE name = g.name) AS y) AS oracle_only
FROM geoms g, parent p
ORDER BY g.name;

-- A collection gives the union of the covers of its components
SELECT geoToQuadbinSet(geometry 'SRID=4326;GEOMETRYCOLLECTION(
    POINT(4.35 50.85), LINESTRING(4.40 50.88, 4.42 50.90),
    POLYGON((4.30 50.80, 4.32 50.80, 4.32 50.82, 4.30 50.82, 4.30 50.80)))',
    12) =
  setUnion(setUnion(
    geoToQuadbinSet(geometry 'SRID=4326;POINT(4.35 50.85)', 12),
    geoToQuadbinSet(geometry 'SRID=4326;LINESTRING(4.40 50.88, 4.42 50.90)',
      12)),
    geoToQuadbinSet(geometry 'SRID=4326;POLYGON((4.30 50.80, 4.32 50.80,
      4.32 50.82, 4.30 50.82, 4.30 50.80))', 12));

-- An empty geometry holds no cell
SELECT geoToQuadbinSet(geometry 'SRID=4326;POINT EMPTY', 10) IS NULL;

-- A resolution outside 0 to 26, a reference system other than lon/lat, a
-- curve, alone or inside a collection, whose cells the walk does not state,
-- and a cover of more cells than a set is built from
/* Errors */
SELECT geoToQuadbinSet(geometry 'SRID=4326;POINT(4.35 50.85)', 27);
SELECT geoToQuadbinSet(geometry 'SRID=4326;POINT(4.35 50.85)', -1);
SELECT geoToQuadbinSet(geometry 'SRID=3857;POINT(4.35 50.85)', 10);
SELECT geoToQuadbinSet(geometry 'SRID=4326;CURVEPOLYGON(CIRCULARSTRING(
  4.30 50.80, 4.40 50.90, 4.30 50.80))', 10);
SELECT geoToQuadbinSet(geometry 'SRID=4326;GEOMETRYCOLLECTION(
  POINT(4.35 50.85), CIRCULARSTRING(4.30 50.80, 4.35 50.85, 4.40 50.80))', 10);
SELECT geoToQuadbinSet(geometry 'SRID=4326;POLYGON((4.30 50.80, 4.45 50.80,
  4.45 50.90, 4.30 50.90, 4.30 50.80))', 26);

-------------------------------------------------------------------------------
-- Boundary
-------------------------------------------------------------------------------

SELECT ST_GeometryType(cellToBoundary(quadbin '48a6227affffffff'));
SELECT round(xMax(stbox(quadbin '48a6227affffffff'))::numeric, 6);
SELECT ST_SRID(cellToBoundary(quadbin '48a6227affffffff'));

-- The centroid point lies inside the cell boundary polygon
SELECT ST_Contains(
  cellToBoundary(quadbin '48a6227affffffff'),
  cellToPoint(quadbin '48a6227affffffff'));

-------------------------------------------------------------------------------
-- Area (square metres)
-------------------------------------------------------------------------------

-- The whole-world z0 cell area
SELECT round(cellArea(quadbin '480fffffffffffff')::numeric, 1);
-- A finer cell has strictly smaller area than a coarser one
SELECT cellArea(quadbin '48a6227affffffff')
  < cellArea(quadbin '48427fffffffffff');

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

-------------------------------------------------------------------------------
-- A cell composed with a time keeps the space it covers
--
-- `stbox(cell)` and `stbox(cell, <time>)` bound the same region; the second
-- adds the period and nothing else, so its X and Y are those of the first.
-------------------------------------------------------------------------------

SELECT round(stbox(quadbin '48a6227affffffff', timestamptz '2001-01-01'), 6);
SELECT round(stbox(quadbin '48a6227affffffff', tstzspan '[2001-01-01, 2001-01-02]'), 6);
SELECT round(stbox(quadbin '48a6227affffffff', timestamptz '2001-01-01')::stbox, 6)
  && round(stbox(quadbin '48a6227affffffff'), 6);
SELECT hasX(stbox(quadbin '48a6227affffffff', timestamptz '2001-01-01')),
  hasT(stbox(quadbin '48a6227affffffff', timestamptz '2001-01-01'));
SELECT SRID(stbox(quadbin '48a6227affffffff', tstzspan '[2001-01-01, 2001-01-02]'));

-------------------------------------------------------------------------------
