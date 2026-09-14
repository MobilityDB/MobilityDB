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

-- The six shared DGGS operations lifted over a temporal value, plus the token.
--
-- Each answer is cross-checked against the static form it lifts: the temporal
-- operation applied to a one-instant value must equal the static operation
-- applied to that instant's cell.

-------------------------------------------------------------------------------
-- Resolution, validity and area
-------------------------------------------------------------------------------

SELECT asText(getResolution(ts2cell '[47c3c3@2001-01-01, 47c3c38705f@2001-01-02]'));
SELECT asText(isValidCell(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]'));
SELECT startValue(getResolution(ts2cell '[47c3c3@2001-01-01]'))
  = getResolution(s2cell '47c3c3');
SELECT round(startValue(cellArea(ts2cell '[47c3c3@2001-01-01]'))::numeric, 6)
  = round(cellArea(s2cell '47c3c3')::numeric, 6);

-------------------------------------------------------------------------------
-- Hierarchy
-------------------------------------------------------------------------------

SELECT asText(cellToParent(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]', 5));
SELECT startValue(cellToParent(ts2cell '[47c3c3@2001-01-01]', 5))
  = cellToParent(s2cell '47c3c3', 5);

-------------------------------------------------------------------------------
-- Centre and boundary
--
-- An S2 cell is a region of the sphere, so the centre is a tgeogpoint and the
-- boundary a tgeography, both in SRID 4326.
-------------------------------------------------------------------------------

SELECT tempSubtype(cellToPoint(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]'));
SELECT tempBasetype(cellToPoint(ts2cell '[47c3c3@2001-01-01]'));
SELECT tempBasetype(cellToBoundary(ts2cell '[47c3c3@2001-01-01]'));
SELECT startValue(cellToPoint(ts2cell '[47c3c3@2001-01-01]'))::geometry
  = cellToPoint(s2cell '47c3c3')::geometry;
SELECT ST_NPoints(startValue(cellToBoundary(ts2cell '[47c3c3@2001-01-01]'))::geometry);
SELECT ST_SRID(startValue(cellToBoundary(ts2cell '[47c3c3@2001-01-01]'))::geometry);

-------------------------------------------------------------------------------
-- The token
-------------------------------------------------------------------------------

SELECT asText(ts2CellToToken(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]'));
SELECT startValue(ts2CellToToken(ts2cell '[47c3c3@2001-01-01]'))
  = s2CellToToken(s2cell '47c3c3');

-------------------------------------------------------------------------------
-- Conversion from a temporal point: ts2cell(tgeogpoint, integer)
--
-- The cell-entry timestamps are interpolated from floating-point crossings
-- whose low digits vary across build configurations, so the cases below state
-- the build-stable invariants and print whole values only where every
-- timestamp is exact.
-------------------------------------------------------------------------------

-- An instant yields the cell holding its position
SELECT ts2cell(tgeogpoint 'Point(4.35 50.85)@2001-01-01', 10);
SELECT startValue(ts2cell(tgeogpoint 'Point(4.35 50.85)@2001-01-01', 10))
  = geoToS2Cell(geography 'Point(4.35 50.85)', 10);

-- A trajectory staying inside one cell holds it over its whole period
SELECT ts2cell(tgeogpoint
  '[Point(4.3500 50.8500)@2001-01-01, Point(4.3501 50.8501)@2001-01-02]', 10);

-- A trajectory that states nothing between its instants holds the cells of
-- those instants, whether discrete or stepwise
SELECT ts2cell(tgeogpoint '{Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02}', 4);
SELECT ts2cell(tgeogpoint
  'Interp=Step;[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02]', 4);

-- A linear trajectory holds every cell it crosses, each entered once, the
-- first holding the position of its start and the last that of its end. The
-- segment below passes from cube face 0 to cube face 1
SELECT numValues(getValues(t)), numInstants(t),
  startValue(t) = geoToS2Cell(geography 'Point(1 1)', 4),
  endValue(t) = geoToS2Cell(geography 'Point(60 -40)', 4)
FROM (SELECT ts2cell(tgeogpoint
  '[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02]', 4) AS t) AS q;

-- At every sampled timestamp the value is the cell holding the position of the
-- trajectory then, over a path from one cube face to the next, a path across
-- the antimeridian, a path beside the pole, a path along the prime meridian,
-- which on cube face 2 is the line v = 0, an edge between cells at every level,
-- so the path meets each next cell at a vertex, and a path along that line over
-- the pole and down the opposite meridian
SELECT count(*) AS instants,
  count(*) FILTER (WHERE valueAtTimestamp(c, t) <>
    geoToS2Cell(valueAtTimestamp(p, t), 8)) AS other_cell
FROM (SELECT p, ts2cell(p, 8) AS c FROM (VALUES
  (tgeogpoint '[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02]'),
  (tgeogpoint '[Point(179 10)@2001-01-01, Point(-179 12)@2001-01-03]'),
  (tgeogpoint '[Point(0 88)@2001-01-01, Point(170 88)@2001-01-03]'),
  (tgeogpoint '[Point(0 86)@2001-01-01, Point(0 89.5)@2001-01-03]'),
  (tgeogpoint '[Point(0 88)@2001-01-01, Point(180 88)@2001-01-03]'))
  AS v(p)) AS q,
  generate_series(timestamptz '2001-01-01 00:17', timestamptz '2001-01-02 23:00',
    interval '37 minutes') AS t
WHERE valueAtTimestamp(p, t) IS NOT NULL;

-- A path along the meridian 90, which cube face 2 maps to the line u = 0, an
-- edge between cells at every level: the path crosses that edge within the
-- rounding of its own position, so the cells it holds do not change there,
-- and at level 16 the value at every sampled timestamp is still the cell
-- holding the position of the trajectory then
SELECT count(*) AS instants,
  count(*) FILTER (WHERE valueAtTimestamp(c, t) <>
    geoToS2Cell(valueAtTimestamp(p, t), 16)) AS other_cell
FROM (SELECT p, ts2cell(p, 16) AS c FROM (VALUES
  (tgeogpoint '[Point(90 89.661652198)@2001-01-01, Point(90 88.678114725)@2001-01-03]'))
  AS v(p)) AS q,
  generate_series(timestamptz '2001-01-01 00:17', timestamptz '2001-01-02 23:00',
    interval '37 minutes') AS t
WHERE valueAtTimestamp(p, t) IS NOT NULL;

-- The last cell holds to the end of the trajectory, so the temporal cell spans
-- the period of the trajectory under an inclusive and an exclusive upper bound
SELECT getTime(ts2cell(p, 10)) = getTime(p) FROM (VALUES
  (tgeogpoint '[Point(4.30 50.80)@2001-01-01, Point(4.40 50.90)@2001-01-02]'),
  (tgeogpoint '[Point(4.30 50.80)@2001-01-01, Point(4.40 50.90)@2001-01-02)'))
  AS v(p);
SELECT endValue(ts2cell(tgeogpoint
  '[Point(4.30 50.80)@2001-01-01, Point(4.40 50.90)@2001-01-02)', 10))
  = geoToS2Cell(geography 'Point(4.40 50.90)', 10);

-- A sequence set yields one sequence per sequence
SELECT numSequences(ts2cell(tgeogpoint
  '{[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02],
  [Point(4.30 50.80)@2001-01-03, Point(4.40 50.90)@2001-01-04]}', 4));

-- A level outside 0 to 30
/* Errors */
SELECT ts2cell(tgeogpoint 'Point(4.35 50.85)@2001-01-01', 31);
SELECT ts2cell(tgeogpoint 'Point(4.35 50.85)@2001-01-01', -1);

-------------------------------------------------------------------------------
