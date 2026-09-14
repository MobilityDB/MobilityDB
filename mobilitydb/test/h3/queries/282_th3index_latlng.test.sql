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

-- §1.1 Lat/Lng conversions — five lifts, all backed by adapters in
-- th3index_latlng.c (geo_to_h3index_cell / h3index_cell_to_point /
-- h3index_cell_to_boundary). Round-trip identities are the most
-- robust assertions because they hold without us hard-coding any
-- specific lat/lng coordinates.
--
-- Test cells:
--   590464338553208831 = res 3 hexagon
--   622236750694711295 = res 10 NYC hexagon

-------------------------------------------------------------------------------
-- cellToPoint — geodetic centroid trajectory
-------------------------------------------------------------------------------

-- Result is non-NULL and is a tgeogpoint.
SELECT cellToPoint(th3index '831c02fffffffff@2001-01-01') IS NOT NULL;
SELECT cellToPoint(th3index '8a2a1072b59ffff@2001-01-01') IS NOT NULL;

-- All four temporal subtypes
SELECT cellToPoint(th3index
  '{831c02fffffffff@2001-01-01, 8a2a1072b59ffff@2001-01-02}') IS NOT NULL;
SELECT cellToPoint(th3index
  '[831c02fffffffff@2001-01-01, 8a2a1072b59ffff@2001-01-02]') IS NOT NULL;

-- Round trip: latlng -> cell at the same resolution gives the original cell
-- back. This holds because cellToLatLng yields the centroid, and
-- latLngToCell maps the centroid to the same cell.
SELECT th3index(
  cellToPoint(th3index '8a2a1072b59ffff@2001-01-01'), 10)
  = th3index '8a2a1072b59ffff@2001-01-01';

-------------------------------------------------------------------------------
-- tgeompoint — planar (SRID 4326) overload
-------------------------------------------------------------------------------

SELECT tgeompoint(th3index
  '831c02fffffffff@2001-01-01') IS NOT NULL;

-------------------------------------------------------------------------------
-- th3index(tgeogpoint, integer)
-------------------------------------------------------------------------------

-- A geodetic point indexed at resolution R yields a cell whose
-- resolution is R.
SELECT getResolution(th3index(
  tgeogpoint 'POINT(-73.96 40.78)@2001-01-01', 9));

-- Sequence form. The densifying conversion resamples the trajectory into the
-- H3 cells its great-circle path crosses. The cell-crossing timestamps are
-- floating-point geometry whose low digits vary across build configurations,
-- so the test asserts the build-stable invariants: the densified sequence
-- preserves the endpoint cells (equal to the direct per-point conversion) and
-- inserts the intermediate cells the path traverses.
WITH densified AS (
  SELECT th3index(
    tgeogpoint '[POINT(-73.96 40.78)@2001-01-01, POINT(2.35 48.86)@2001-01-02]',
    9) AS t
)
SELECT startValue(t) = latLngToCell(geometry 'SRID=4326;POINT(-73.96 40.78)', 9) AS start_cell_preserved,
       endValue(t)   = latLngToCell(geometry 'SRID=4326;POINT(2.35 48.86)', 9)   AS end_cell_preserved,
       numValues(getValues(t)) > 1000 AS path_densified
FROM densified;

-------------------------------------------------------------------------------
-- th3index(tgeompoint, integer)
--
-- The tgeompoint overload requires SRID 4326; the adapter is
-- expected to raise on mismatch.
-------------------------------------------------------------------------------

SELECT getResolution(th3index(
  tgeompoint 'SRID=4326;POINT(-73.96 40.78)@2001-01-01', 9));

-- Sequence input on the planar (SRID 4326) overload exercises the densify
-- walker through the SRID-guarded first lookup; assert the build-stable
-- endpoint cells equal the direct per-point conversion.
WITH d AS (
  SELECT th3index(
    tgeompoint 'SRID=4326;[POINT(-73.96 40.78)@2001-01-01, POINT(-73.90 40.80)@2001-01-02]', 7) AS t
)
SELECT startValue(t) = latLngToCell(geometry 'SRID=4326;POINT(-73.96 40.78)', 7) AS start_cell_preserved,
       endValue(t)   = latLngToCell(geometry 'SRID=4326;POINT(-73.90 40.80)', 7) AS end_cell_preserved
FROM d;

-- The last cell holds to the end of the trajectory, so the temporal cell spans
-- the period of the trajectory, across several cells and under an exclusive
-- upper bound alike, for both overloads.
SELECT getTime(th3index(p, 10)) = getTime(p) FROM (VALUES
  (tgeompoint 'SRID=4326;[Point(4.30 50.80)@2001-01-01, Point(4.31 50.81)@2001-01-02]'),
  (tgeompoint 'SRID=4326;[Point(4.30 50.80)@2001-01-01, Point(4.31 50.81)@2001-01-02)'))
  AS v(p);
SELECT getTime(th3index(p, 10)) = getTime(p) FROM (VALUES
  (tgeogpoint '[Point(4.30 50.80)@2001-01-01, Point(4.31 50.81)@2001-01-02]'),
  (tgeogpoint '[Point(4.30 50.80)@2001-01-01, Point(4.31 50.81)@2001-01-02)'))
  AS v(p);

-- A trajectory staying inside one cell holds it over its whole period
SELECT asText(th3index(tgeompoint
  'SRID=4326;[Point(4.3000 50.8000)@2001-01-01, Point(4.3001 50.8001)@2001-01-02]', 7));
SELECT asText(th3index(tgeogpoint
  '[Point(4.3000 50.8000)@2001-01-01, Point(4.3001 50.8001)@2001-01-02]', 7));

-- Under an exclusive upper bound, the trajectory holds the cell of its final
-- position from the time it enters it until the end
SELECT endValue(th3index(tgeompoint
  'SRID=4326;[Point(4.30 50.80)@2001-01-01, Point(4.31 50.81)@2001-01-02)', 10))
  = latLngToCell(geometry 'SRID=4326;POINT(4.31 50.81)', 10);

-- Mismatched SRID — must error once the adapter validates
/* Errors */
SELECT th3index(
  tgeompoint 'SRID=3857;POINT(-73.96 40.78)@2001-01-01', 9);

-- Mismatched SRID on the densify (sequence) path must error too
/* Errors */
SELECT th3index(
  tgeompoint 'SRID=3857;[POINT(-73.96 40.78)@2001-01-01, POINT(-73.90 40.80)@2001-01-02]', 7);

-------------------------------------------------------------------------------
-- cellToBoundary — per-instant polygon as tgeography
-------------------------------------------------------------------------------

SELECT cellToBoundary(th3index '831c02fffffffff@2001-01-01') IS NOT NULL;
SELECT cellToBoundary(th3index '8a2a1072b59ffff@2001-01-01') IS NOT NULL;

SELECT cellToBoundary(th3index
  '[831c02fffffffff@2001-01-01, 8a2a1072b59ffff@2001-01-02]') IS NOT NULL;

-------------------------------------------------------------------------------
-- Casts — th3index :: tgeogpoint / tgeompoint (sugar over the conversions above)
-------------------------------------------------------------------------------

SELECT (th3index '831c02fffffffff@2001-01-01')::tgeogpoint IS NOT NULL;
SELECT (th3index
  '[831c02fffffffff@2001-01-01, 8a2a1072b59ffff@2001-01-02]')::tgeogpoint
  IS NOT NULL;
-- Equivalence with the explicit function call
SELECT (th3index '831c02fffffffff@2001-01-01')::tgeogpoint
  ~= cellToPoint(th3index '831c02fffffffff@2001-01-01');

SELECT (th3index '831c02fffffffff@2001-01-01')::tgeompoint IS NOT NULL;
SELECT (th3index
  '[831c02fffffffff@2001-01-01, 8a2a1072b59ffff@2001-01-02]')::tgeompoint
  IS NOT NULL;
SELECT (th3index '831c02fffffffff@2001-01-01')::tgeompoint
  ~= tgeompoint(th3index '831c02fffffffff@2001-01-01');

-------------------------------------------------------------------------------
-- A moving trajectory holds every cell it crosses
-------------------------------------------------------------------------------

-- A trajectory that states nothing between its instants holds the cells of
-- those instants alone, so a two-instant discrete value answers at most two.
SELECT numInstants(th3index(tgeompoint
  'SRID=4326;{Point(4.30 50.80)@2001-01-01, Point(4.31 50.81)@2001-01-02}',
  10));

-- It keeps its interpolation, discrete or stepwise, since it states nothing
-- between its instants
SELECT interp(th3index(tgeompoint
  'SRID=4326;{Point(4.30 50.80)@2001-01-01, Point(4.31 50.81)@2001-01-02}',
  10));
SELECT interp(th3index(tgeogpoint
  '{Point(4.30 50.80)@2001-01-01, Point(4.31 50.81)@2001-01-02}', 10));
SELECT interp(th3index(setSRID(tgeompoint
  'Interp=Step;[Point(4.30 50.80)@2001-01-01, Point(4.31 50.81)@2001-01-02]',
  4326), 10));

-- One that moves between them holds every cell along the way, and the count
-- is the cells the segment enters rather than a function of any sampling
-- rate. A cell is entered where the path leaves the previous one, so the
-- answer counts boundary crossings.
SELECT numValues(getValues(th3index(tgeompoint
  'SRID=4326;[Point(4.30 50.80)@2001-01-01, Point(4.31 50.81)@2001-01-02]',
  10)));

-- The walk enters each cell once, so the instants are the distinct cells of
-- the cover and the closing instant that holds the last one to the end.
SELECT numInstants(t) = numValues(getValues(t)) + 1 FROM (SELECT th3index(tgeompoint
  'SRID=4326;[Point(4.30 50.80)@2001-01-01, Point(4.31 50.81)@2001-01-02]',
  10) AS t) AS q;

-- A geodetic trajectory holds the cell of its own position at every instant
-- at the finest resolution too, where a cell edge is under a metre long
SELECT count(*) FILTER (WHERE valueAtTimestamp(c, t) IS NOT NULL) AS instants,
  count(*) FILTER (WHERE valueAtTimestamp(c, t) <>
    startValue(th3index(atTime(p, t), 15))) AS other_cell
FROM (SELECT p, th3index(p, 15) AS c FROM (VALUES (tgeogpoint
  '[Point(37.1124 -29.7116)@2001-01-01, Point(37.1124305 -29.7116247)@2001-01-03]'))
  AS v(p)) AS q, generate_series(timestamptz '2001-01-01 00:17',
    '2001-01-02 23:43', interval '37 minutes') AS t;

-- A geodetic trajectory moves along the great circle between its instants, so
-- one crossing the antimeridian enters the cells along its shortest route.
-- The planar trajectory between the same positions moves along the straight
-- line in longitude and latitude, through every longitude between them.
SELECT numInstants(th3index(tgeogpoint
  '[Point(179.5 0)@2001-01-01, Point(-179.5 0)@2001-01-02]', 3));
SELECT numInstants(th3index(tgeompoint
  'SRID=4326;[Point(179.5 0)@2001-01-01, Point(-179.5 0)@2001-01-02]', 3));

-- At every instant a geodetic trajectory holds the cell of its own position:
-- across the antimeridian, and along the arc between two positions on the
-- 70th parallel, which rises toward the pole between them.
SELECT count(*) FILTER (WHERE valueAtTimestamp(c, t) IS NOT NULL) AS instants,
  count(*) FILTER (WHERE valueAtTimestamp(c, t) <>
    startValue(th3index(atTime(p, t), 3))) AS other_cell
FROM (SELECT p, th3index(p, 3) AS c FROM (VALUES
  (tgeogpoint '[Point(179.5 0)@2001-01-01, Point(-179.5 0)@2001-01-02]'),
  (tgeogpoint '[Point(-60 70)@2001-01-01, Point(60 70)@2001-01-03]')) AS v(p))
  AS q, generate_series(timestamptz '2001-01-01 00:17',
    '2001-01-02 23:43', interval '37 minutes') AS t;

-- So does one moving along a meridian, whose bearing is due north
SELECT count(*) FILTER (WHERE valueAtTimestamp(c, t) IS NOT NULL) AS instants,
  count(*) FILTER (WHERE valueAtTimestamp(c, t) <>
    startValue(th3index(atTime(p, t), 9))) AS other_cell
FROM (SELECT p, th3index(p, 9) AS c FROM (VALUES
  (tgeogpoint '[Point(0 -79.269)@2001-01-01, Point(0 -79.239)@2001-01-03]'))
  AS v(p)) AS q, generate_series(timestamptz '2001-01-01 00:17',
    '2001-01-02 23:43', interval '37 minutes') AS t;

-------------------------------------------------------------------------------
