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

-- tquadbin accessors and lifted cell operations: value accessors, resolution,
-- validity, parent, centroid point, boundary, area, quadkey, and the
-- tquadbin -> tgeompoint cast. Each lifts its static kernel over the time axis.

-------------------------------------------------------------------------------
-- Value accessors
-------------------------------------------------------------------------------

SELECT startValue(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');
SELECT endValue(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');
SELECT valueN(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}', 2);
SELECT getValues(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');
SELECT valueAtTimestamp(tquadbin '[480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-03]', '2001-01-01');

-------------------------------------------------------------------------------
-- Resolution + validity (lifted)
-------------------------------------------------------------------------------

SELECT getResolution(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');
SELECT isValidCell(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');

-------------------------------------------------------------------------------
-- Hierarchy (lifted parent at a constant resolution)
-------------------------------------------------------------------------------

SELECT cellToParent(tquadbin '{48427fffffffffff@2001-01-01, 48a6227affffffff@2001-01-02}', 0);

-- Every instant's parent at res 0 is the z0 world cell
SELECT getValues(cellToParent(
  tquadbin '{48427fffffffffff@2001-01-01, 48a6227affffffff@2001-01-02}', 0))
  = quadbinset '{480fffffffffffff}';

-------------------------------------------------------------------------------
-- Centroid point / boundary (lifted)
-------------------------------------------------------------------------------

SELECT cellToPoint(tquadbin '{48a6227affffffff@2001-01-01}');
SELECT asText(cellToPoint(tquadbin '{48a6227affffffff@2001-01-01}'), 6);
SELECT cellToBoundary(tquadbin '{48a6227affffffff@2001-01-01}') IS NOT NULL;

-------------------------------------------------------------------------------
-- Area (lifted, tfloat)
-------------------------------------------------------------------------------

SELECT round(startValue(cellArea(tquadbin '{480fffffffffffff@2001-01-01}'))::numeric, 1);

-------------------------------------------------------------------------------
-- Quadkey (lifted, ttext)
-------------------------------------------------------------------------------

SELECT tquadbinCellToQuadkey(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');

-------------------------------------------------------------------------------
-- Convenience cast: tquadbin :: tgeompoint  (reuses cell_to_point centroid)
-------------------------------------------------------------------------------

SELECT (tquadbin '{48a6227affffffff@2001-01-01}')::tgeompoint
  = cellToPoint(tquadbin '{48a6227affffffff@2001-01-01}');
SELECT asText((tquadbin '{48a6227affffffff@2001-01-01}')::tgeompoint, 6);

-------------------------------------------------------------------------------
-- Conversion from a temporal point: tquadbin(tgeompoint, integer)
--
-- The cell-entry timestamps are interpolated from floating-point crossings
-- whose low digits vary across build configurations, so the cases below state
-- the build-stable invariants and print whole values only where every
-- timestamp is exact.
-------------------------------------------------------------------------------

-- An instant yields the cell holding its position
SELECT tquadbin(tgeompoint 'SRID=4326;Point(4.35 50.85)@2001-01-01', 10);
SELECT startValue(tquadbin(tgeompoint 'SRID=4326;Point(4.35 50.85)@2001-01-01', 10))
  = geoToQuadbinCell(geometry 'SRID=4326;Point(4.35 50.85)', 10);

-- A trajectory staying inside one cell holds it over its whole period
SELECT tquadbin(tgeompoint
  'SRID=4326;[Point(4.3500 50.8500)@2001-01-01, Point(4.3501 50.8501)@2001-01-02]', 10);

-- A trajectory that states nothing between its instants holds the cells of
-- those instants, whether discrete or stepwise
SELECT tquadbin(tgeompoint
  'SRID=4326;{Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02}', 4);
SELECT tquadbin(setSRID(tgeompoint
  'Interp=Step;[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02]', 4326), 4);

-- A linear trajectory holds every cell it crosses. The segment below starts in
-- the zoom-4 tile of column 8 and row 7 and ends in the tile of column 10 and
-- row 9, so it enters two more columns and two more rows: five cells, each
-- entered once, and the closing instant holding the last one to the end.
SELECT numValues(getValues(t)), numInstants(t),
  startValue(t) = geoToQuadbinCell(geometry 'SRID=4326;Point(1 1)', 4),
  endValue(t) = geoToQuadbinCell(geometry 'SRID=4326;Point(60 -40)', 4)
FROM (SELECT tquadbin(tgeompoint
  'SRID=4326;[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02]', 4) AS t) AS q;

-- At every sampled timestamp the value is the cell holding the position of the
-- trajectory then
SELECT count(*) AS instants,
  count(*) FILTER (WHERE valueAtTimestamp(c, t) <>
    geoToQuadbinCell(valueAtTimestamp(p, t), 4)) AS other_cell
FROM (SELECT p, tquadbin(p, 4) AS c FROM (VALUES
  (tgeompoint 'SRID=4326;[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02]'),
  (tgeompoint 'SRID=4326;[Point(-170 70)@2001-01-01, Point(170 -70)@2001-01-03]'))
  AS v(p)) AS q,
  generate_series(timestamptz '2001-01-01 00:17', timestamptz '2001-01-02 23:00',
    interval '37 minutes') AS t
WHERE valueAtTimestamp(p, t) IS NOT NULL;

-- The last cell holds to the end of the trajectory, so the temporal cell spans
-- the period of the trajectory under an inclusive and an exclusive upper bound
SELECT getTime(tquadbin(p, 10)) = getTime(p) FROM (VALUES
  (tgeompoint 'SRID=4326;[Point(4.30 50.80)@2001-01-01, Point(4.40 50.90)@2001-01-02]'),
  (tgeompoint 'SRID=4326;[Point(4.30 50.80)@2001-01-01, Point(4.40 50.90)@2001-01-02)'))
  AS v(p);
SELECT endValue(tquadbin(tgeompoint
  'SRID=4326;[Point(4.30 50.80)@2001-01-01, Point(4.40 50.90)@2001-01-02)', 10))
  = geoToQuadbinCell(geometry 'SRID=4326;Point(4.40 50.90)', 10);

-- A tile holds its west and north boundaries, so the point where four tiles
-- meet belongs to the one east of the meridian and south of the parallel
-- meeting there. A path through that point from the south-west passes through
-- that tile at the crossing and then enters the tile diagonally across, and
-- one from the south-east crosses straight from its own tile, which holds the
-- point, into the tile diagonally across.
SELECT tquadbin(tgeompoint
  'SRID=4326;[Point(-135 -10)@2001-01-01, Point(-45 10)@2001-01-03]', 2);
SELECT valueAtTimestamp(tquadbin(tgeompoint
  'SRID=4326;[Point(-135 -10)@2001-01-01, Point(-45 10)@2001-01-03]', 2),
  timestamptz '2001-01-02') = geoToQuadbinCell(geometry 'SRID=4326;Point(-90 0)', 2);
SELECT tquadbin(tgeompoint
  'SRID=4326;[Point(-45 -10)@2001-01-01, Point(-135 10)@2001-01-03]', 2);

-- A sequence set yields one sequence per sequence
SELECT numSequences(tquadbin(tgeompoint
  'SRID=4326;{[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02],
  [Point(4.30 50.80)@2001-01-03, Point(4.40 50.90)@2001-01-04]}', 4));

-- A reference system other than lon/lat, and a resolution outside 0 to 26
/* Errors */
SELECT tquadbin(tgeompoint 'SRID=3857;Point(4.35 50.85)@2001-01-01', 10);
SELECT tquadbin(tgeompoint
  'SRID=3857;[Point(4.35 50.85)@2001-01-01, Point(4.36 50.86)@2001-01-02]', 10);
SELECT tquadbin(tgeompoint 'SRID=4326;Point(4.35 50.85)@2001-01-01', 27);
SELECT tquadbin(tgeompoint 'SRID=4326;Point(4.35 50.85)@2001-01-01', -1);

-------------------------------------------------------------------------------
