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

-- Sequence form. We extract startValue / endValue rather than rendering
-- the full sequence: the intermediate cell-crossing timestamps depend on
-- floating-point precision in the great-circle-to-icosahedron projection,
-- which varies across builds (Release vs Debug, libgeos minor versions,
-- coverage instrumentation). The startValue / endValue endpoints are
-- bit-stable: both must be the requested resolution.
SELECT startValue(h3_get_resolution(h3_latlng_to_cell(
  tgeogpoint '[POINT(-73.96 40.78)@2001-01-01, POINT(2.35 48.86)@2001-01-02]',
  9))) = 9
   AND endValue(h3_get_resolution(h3_latlng_to_cell(
  tgeogpoint '[POINT(-73.96 40.78)@2001-01-01, POINT(2.35 48.86)@2001-01-02]',
  9))) = 9 AS endpoints_at_requested_resolution;

-------------------------------------------------------------------------------
-- h3_latlng_to_cell(tgeompoint, integer)
--
-- The tgeompoint overload requires SRID 4326; the adapter is
-- expected to raise on mismatch.
-------------------------------------------------------------------------------

SELECT h3_get_resolution(h3_latlng_to_cell(
  tgeompoint 'SRID=4326;POINT(-73.96 40.78)@2001-01-01', 9));

-- Mismatched SRID — must error once the adapter validates
/* Errors */
SELECT h3_latlng_to_cell(
  tgeompoint 'SRID=3857;POINT(-73.96 40.78)@2001-01-01', 9);

-------------------------------------------------------------------------------
-- h3_cell_to_boundary — per-instant polygon as tgeography
-------------------------------------------------------------------------------

SELECT h3_cell_to_boundary(th3index '590464338553208831@2001-01-01') IS NOT NULL;
SELECT h3_cell_to_boundary(th3index '622236750694711295@2001-01-01') IS NOT NULL;

SELECT h3_cell_to_boundary(th3index
  '[590464338553208831@2001-01-01, 622236750694711295@2001-01-02]') IS NOT NULL;

-------------------------------------------------------------------------------
