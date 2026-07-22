-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- §1.1 Lat/Lng conversions — five lifts, all backed by adapters in
-- th3index_latlng.c (geo_to_h3index_cell / h3_cell_to_geompoint /
-- h3_cell_to_geom). Round-trip identities are the most
-- robust assertions because they hold without us hard-coding any
-- specific lat/lng coordinates.
--
-- Test cells:
--   590464338553208831 = res 3 hexagon
--   622236750694711295 = res 10 NYC hexagon

-------------------------------------------------------------------------------
-- th3CellToLatlng — geodetic centroid trajectory
-------------------------------------------------------------------------------

-- Result is non-NULL and is a tgeogpoint.
SELECT th3CellToLatlng(th3index '590464338553208831@2001-01-01') IS NOT NULL;
SELECT th3CellToLatlng(th3index '622236750694711295@2001-01-01') IS NOT NULL;

-- All four temporal subtypes
SELECT th3CellToLatlng(th3index
  '{590464338553208831@2001-01-01, 622236750694711295@2001-01-02}') IS NOT NULL;
SELECT th3CellToLatlng(th3index
  '[590464338553208831@2001-01-01, 622236750694711295@2001-01-02]') IS NOT NULL;

-- Round trip: latlng -> cell at the same resolution gives the original cell
-- back. This holds because cellToLatLng yields the centroid, and
-- latLngToCell maps the centroid to the same cell.
SELECT th3index(
  th3CellToLatlng(th3index '622236750694711295@2001-01-01'), 10)
  = th3index '622236750694711295@2001-01-01';

-------------------------------------------------------------------------------
-- th3CellToLatlngTgeompoint — planar (SRID 4326) overload
-------------------------------------------------------------------------------

SELECT th3CellToLatlngTgeompoint(th3index
  '590464338553208831@2001-01-01') IS NOT NULL;

-------------------------------------------------------------------------------
-- th3index(tgeogpoint, integer)
-------------------------------------------------------------------------------

-- A geodetic point indexed at resolution R yields a cell whose
-- resolution is R.
SELECT th3GetResolution(th3index(
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
SELECT startValue(t) = geoToH3Cell(geometry 'SRID=4326;POINT(-73.96 40.78)', 9) AS start_cell_preserved,
       endValue(t)   = geoToH3Cell(geometry 'SRID=4326;POINT(2.35 48.86)', 9)   AS end_cell_preserved,
       numValues(getValues(t)) > 1000 AS path_densified
FROM densified;

-------------------------------------------------------------------------------
-- th3index(tgeompoint, integer)
--
-- The tgeompoint overload requires SRID 4326; the adapter is
-- expected to raise on mismatch.
-------------------------------------------------------------------------------

SELECT th3GetResolution(th3index(
  tgeompoint 'SRID=4326;POINT(-73.96 40.78)@2001-01-01', 9));

-- Sequence input on the planar (SRID 4326) overload exercises the densify
-- walker through the SRID-guarded first lookup; assert the build-stable
-- endpoint cells equal the direct per-point conversion.
WITH d AS (
  SELECT th3index(
    tgeompoint 'SRID=4326;[POINT(-73.96 40.78)@2001-01-01, POINT(-73.90 40.80)@2001-01-02]', 7) AS t
)
SELECT startValue(t) = geoToH3Cell(geometry 'SRID=4326;POINT(-73.96 40.78)', 7) AS start_cell_preserved,
       endValue(t)   = geoToH3Cell(geometry 'SRID=4326;POINT(-73.90 40.80)', 7) AS end_cell_preserved
FROM d;

-- Mismatched SRID — must error once the adapter validates
/* Errors */
SELECT th3index(
  tgeompoint 'SRID=3857;POINT(-73.96 40.78)@2001-01-01', 9);

-- Mismatched SRID on the densify (sequence) path must error too
/* Errors */
SELECT th3index(
  tgeompoint 'SRID=3857;[POINT(-73.96 40.78)@2001-01-01, POINT(-73.90 40.80)@2001-01-02]', 7);

-------------------------------------------------------------------------------
-- th3CellToBoundary — per-instant polygon as tgeography
-------------------------------------------------------------------------------

SELECT th3CellToBoundary(th3index '590464338553208831@2001-01-01') IS NOT NULL;
SELECT th3CellToBoundary(th3index '622236750694711295@2001-01-01') IS NOT NULL;

SELECT th3CellToBoundary(th3index
  '[590464338553208831@2001-01-01, 622236750694711295@2001-01-02]') IS NOT NULL;

-------------------------------------------------------------------------------
-- Casts — th3index :: tgeogpoint / tgeompoint (sugar over the conversions above)
-------------------------------------------------------------------------------

SELECT (th3index '590464338553208831@2001-01-01')::tgeogpoint IS NOT NULL;
SELECT (th3index
  '[590464338553208831@2001-01-01, 622236750694711295@2001-01-02]')::tgeogpoint
  IS NOT NULL;
-- Equivalence with the explicit function call
SELECT (th3index '590464338553208831@2001-01-01')::tgeogpoint
  ~= th3CellToLatlng(th3index '590464338553208831@2001-01-01');

SELECT (th3index '590464338553208831@2001-01-01')::tgeompoint IS NOT NULL;
SELECT (th3index
  '[590464338553208831@2001-01-01, 622236750694711295@2001-01-02]')::tgeompoint
  IS NOT NULL;
SELECT (th3index '590464338553208831@2001-01-01')::tgeompoint
  ~= th3CellToLatlngTgeompoint(th3index '590464338553208831@2001-01-01');

-------------------------------------------------------------------------------
