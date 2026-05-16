-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- Regression tests for §1.1 lat/lng — one scalar per query.
--
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_th3index_known;
CREATE TABLE tbl_th3index_known(k int PRIMARY KEY, temp th3index);
INSERT INTO tbl_th3index_known VALUES
  (1, th3index '590464338553208831@2001-01-01'),  -- res 3 hex
  (2, th3index '590464201114255359@2001-01-01'),  -- res 3 pentagon
  (3, th3index '595812165542215679@2001-01-01'),  -- res 4 pentagon
  (4, th3index '612544986753269759@2001-01-01'),  -- res 8 hex
  (5, th3index '612544986761658367@2001-01-01'),  -- res 8 hex
  (6, th3index '622236750694711295@2001-01-01');  -- res 10 NYC hex

-------------------------------------------------------------------------------
-- h3_cell_to_latlng (geodetic)
-------------------------------------------------------------------------------

-- All rows produce a non-null tgeogpoint
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE h3_cell_to_latlng(temp) IS NOT NULL;

-- Round trip via h3_latlng_to_cell at the original resolution: identity
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE h3_latlng_to_cell(h3_cell_to_latlng(temp),
          startValue(h3_get_resolution(temp))) <> temp;

-------------------------------------------------------------------------------
-- h3_cell_to_latlng_tgeompoint (planar overload)
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_th3index_known
  WHERE h3_cell_to_latlng_tgeompoint(temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- h3_latlng_to_cell(tgeogpoint, integer)
-------------------------------------------------------------------------------

-- A canonical NYC point at resolution 9 yields a cell at res 9
SELECT startValue(h3_get_resolution(h3_latlng_to_cell(
    tgeogpoint 'POINT(-73.96 40.78)@2001-01-01', 9))) = 9;

-- All eight Cartesian-quadrant lat/lng pairs index successfully at res 5
SELECT COUNT(*) FROM (
  VALUES
    (tgeogpoint 'POINT(0 0)@2001-01-01'),
    (tgeogpoint 'POINT(45 0)@2001-01-01'),
    (tgeogpoint 'POINT(-45 0)@2001-01-01'),
    (tgeogpoint 'POINT(0 45)@2001-01-01'),
    (tgeogpoint 'POINT(0 -45)@2001-01-01'),
    (tgeogpoint 'POINT(120 30)@2001-01-01'),
    (tgeogpoint 'POINT(-120 -30)@2001-01-01'),
    (tgeogpoint 'POINT(180 0)@2001-01-01')
  ) AS pts(p)
  WHERE h3_latlng_to_cell(p, 5) IS NOT NULL;

-------------------------------------------------------------------------------
-- h3_latlng_to_cell(tgeompoint, integer)  — SRID 4326
-------------------------------------------------------------------------------

SELECT startValue(h3_get_resolution(h3_latlng_to_cell(
    tgeompoint 'SRID=4326;POINT(-73.96 40.78)@2001-01-01', 9))) = 9;

-------------------------------------------------------------------------------
-- h3_cell_to_boundary
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_th3index_known
  WHERE h3_cell_to_boundary(temp) IS NOT NULL;

-------------------------------------------------------------------------------

DROP TABLE tbl_th3index_known;

-------------------------------------------------------------------------------
