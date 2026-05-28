-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- §1.1 Lat/Lng conversions — five lifts, all backed by adapters in
-- h3_adapter.c (h3_gs_point_to_cell / h3_cell_to_gs_point /
-- h3_cell_to_gs_boundary). Round-trip identities are the most
-- robust assertions because they hold without us hard-coding any
-- specific lat/lng coordinates.
--
-- Test cells:
--   590464338553208831 = res 3 hexagon
--   622236750694711295 = res 10 NYC hexagon

-------------------------------------------------------------------------------
-- h3_cell_to_latlng — geodetic centroid trajectory
-------------------------------------------------------------------------------

-- Result is non-NULL and is a tgeogpoint.
SELECT h3_cell_to_latlng(th3index '590464338553208831@2001-01-01') IS NOT NULL;
SELECT h3_cell_to_latlng(th3index '622236750694711295@2001-01-01') IS NOT NULL;

-- All four temporal subtypes
SELECT h3_cell_to_latlng(th3index
  '{590464338553208831@2001-01-01, 622236750694711295@2001-01-02}') IS NOT NULL;
SELECT h3_cell_to_latlng(th3index
  '[590464338553208831@2001-01-01, 622236750694711295@2001-01-02]') IS NOT NULL;

-- Round trip: latlng -> cell at the same resolution gives the original cell
-- back. This holds because cellToLatLng yields the centroid, and
-- latLngToCell maps the centroid to the same cell.
SELECT h3_latlng_to_cell(
  h3_cell_to_latlng(th3index '622236750694711295@2001-01-01'), 10)
  = th3index '622236750694711295@2001-01-01';

-------------------------------------------------------------------------------
-- h3_cell_to_latlng_tgeompoint — planar (SRID 4326) overload
-------------------------------------------------------------------------------

SELECT h3_cell_to_latlng_tgeompoint(th3index
  '590464338553208831@2001-01-01') IS NOT NULL;

-------------------------------------------------------------------------------
-- h3_latlng_to_cell(tgeogpoint, integer)
-------------------------------------------------------------------------------

-- A geodetic point indexed at resolution R yields a cell whose
-- resolution is R.
SELECT h3_get_resolution(h3_latlng_to_cell(
  tgeogpoint 'POINT(-73.96 40.78)@2001-01-01', 9));

-- Sequence form. The densifying conversion resamples the trajectory into the
-- H3 cells its great-circle path crosses. The cell-crossing timestamps are
-- floating-point geometry whose low digits vary across build configurations,
-- so the test asserts the build-stable invariants: the densified sequence
-- preserves the endpoint cells (equal to the direct per-point conversion) and
-- inserts the intermediate cells the path traverses.
WITH densified AS (
  SELECT h3_latlng_to_cell(
    tgeogpoint '[POINT(-73.96 40.78)@2001-01-01, POINT(2.35 48.86)@2001-01-02]',
    9) AS t
)
SELECT startValue(t) = geoToH3Cell(geometry 'SRID=4326;POINT(-73.96 40.78)', 9) AS start_cell_preserved,
       endValue(t)   = geoToH3Cell(geometry 'SRID=4326;POINT(2.35 48.86)', 9)   AS end_cell_preserved,
       numValues(getValues(t)) > 1000 AS path_densified
FROM densified;

-------------------------------------------------------------------------------
-- h3_latlng_to_cell(tgeompoint, integer)
--
-- The tgeompoint overload requires SRID 4326; the adapter is
-- expected to raise on mismatch.
-------------------------------------------------------------------------------

SELECT h3_get_resolution(h3_latlng_to_cell(
  tgeompoint 'SRID=4326;POINT(-73.96 40.78)@2001-01-01', 9));

-- Sequence input on the planar (SRID 4326) overload exercises the densify
-- walker through the SRID-guarded first lookup; assert the build-stable
-- endpoint cells equal the direct per-point conversion.
WITH d AS (
  SELECT h3_latlng_to_cell(
    tgeompoint 'SRID=4326;[POINT(-73.96 40.78)@2001-01-01, POINT(-73.90 40.80)@2001-01-02]', 7) AS t
)
SELECT startValue(t) = geoToH3Cell(geometry 'SRID=4326;POINT(-73.96 40.78)', 7) AS start_cell_preserved,
       endValue(t)   = geoToH3Cell(geometry 'SRID=4326;POINT(-73.90 40.80)', 7) AS end_cell_preserved
FROM d;

-- Mismatched SRID — must error once the adapter validates
/* Errors */
SELECT h3_latlng_to_cell(
  tgeompoint 'SRID=3857;POINT(-73.96 40.78)@2001-01-01', 9);

-- Mismatched SRID on the densify (sequence) path must error too
/* Errors */
SELECT h3_latlng_to_cell(
  tgeompoint 'SRID=3857;[POINT(-73.96 40.78)@2001-01-01, POINT(-73.90 40.80)@2001-01-02]', 7);

-------------------------------------------------------------------------------
-- h3_cell_to_boundary — per-instant polygon as tgeography
-------------------------------------------------------------------------------

SELECT h3_cell_to_boundary(th3index '590464338553208831@2001-01-01') IS NOT NULL;
SELECT h3_cell_to_boundary(th3index '622236750694711295@2001-01-01') IS NOT NULL;

SELECT h3_cell_to_boundary(th3index
  '[590464338553208831@2001-01-01, 622236750694711295@2001-01-02]') IS NOT NULL;

-------------------------------------------------------------------------------
