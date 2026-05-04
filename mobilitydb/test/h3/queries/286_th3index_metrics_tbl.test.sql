-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- Regression tests for §1.8 metrics — one scalar per query.
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

DROP TABLE IF EXISTS tbl_tgeogpoint_known;
CREATE TABLE tbl_tgeogpoint_known(k int PRIMARY KEY, p tgeogpoint);
INSERT INTO tbl_tgeogpoint_known VALUES
  (1, tgeogpoint 'POINT(-73.96 40.78)@2001-01-01'),
  (2, tgeogpoint 'POINT(2.35 48.86)@2001-01-01'),
  (3, tgeogpoint 'POINT(151.21 -33.87)@2001-01-01'),
  (4, tgeogpoint 'POINT(0 0)@2001-01-01');

-------------------------------------------------------------------------------
-- h3_cell_area
-------------------------------------------------------------------------------

-- Area is positive for every cell, in any unit.
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE startValue(h3_cell_area(temp, 'km2')) <= 0;

SELECT COUNT(*) FROM tbl_th3index_known
  WHERE startValue(h3_cell_area(temp, 'm2')) <= 0;

-- Area in m² is exactly 1e6 × area in km² for every cell (within a tight tolerance).
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE abs(startValue(h3_cell_area(temp, 'm2'))
          - 1e6 * startValue(h3_cell_area(temp, 'km2')))
        > 1e-3 * startValue(h3_cell_area(temp, 'm2'));

-- Coarser cells are larger: row 1 (res 3) > row 6 (res 10).
SELECT startValue(h3_cell_area((SELECT temp FROM tbl_th3index_known WHERE k = 1), 'km2'))
     > startValue(h3_cell_area((SELECT temp FROM tbl_th3index_known WHERE k = 6), 'km2'));

-------------------------------------------------------------------------------
-- h3_edge_length
-------------------------------------------------------------------------------

-- Edge length is positive for every edge built from neighbouring fixture rows.
SELECT COUNT(*) FROM (
  SELECT h3_cells_to_directed_edge(
    (SELECT temp FROM tbl_th3index_known WHERE k = 4),
    (SELECT temp FROM tbl_th3index_known WHERE k = 5)) AS e
  ) AS e
  WHERE startValue(h3_edge_length(e, 'km')) <= 0;

-- Edge length in m is 1000 × length in km.
SELECT abs(
    startValue(h3_edge_length(h3_cells_to_directed_edge(
      (SELECT temp FROM tbl_th3index_known WHERE k = 4),
      (SELECT temp FROM tbl_th3index_known WHERE k = 5)), 'm'))
    - 1000 * startValue(h3_edge_length(h3_cells_to_directed_edge(
      (SELECT temp FROM tbl_th3index_known WHERE k = 4),
      (SELECT temp FROM tbl_th3index_known WHERE k = 5)), 'km')))
  < 1e-6;

-------------------------------------------------------------------------------
-- h3_great_circle_distance
-------------------------------------------------------------------------------

-- Distance from a point to itself is 0.
SELECT COUNT(*) FROM tbl_tgeogpoint_known
  WHERE startValue(h3_great_circle_distance(p, p, 'km')) <> 0;

-- Distance is positive between any two distinct points.
SELECT COUNT(*) FROM tbl_tgeogpoint_known t1, tbl_tgeogpoint_known t2
  WHERE t1.k <> t2.k
    AND startValue(h3_great_circle_distance(t1.p, t2.p, 'km')) <= 0;

-- Distance is symmetric.
SELECT COUNT(*) FROM tbl_tgeogpoint_known t1, tbl_tgeogpoint_known t2
  WHERE t1.k < t2.k
    AND abs(startValue(h3_great_circle_distance(t1.p, t2.p, 'km'))
          - startValue(h3_great_circle_distance(t2.p, t1.p, 'km')))
        > 1e-6;

-- Distance in m = 1000 × distance in km.
SELECT COUNT(*) FROM tbl_tgeogpoint_known t1, tbl_tgeogpoint_known t2
  WHERE t1.k < t2.k
    AND abs(startValue(h3_great_circle_distance(t1.p, t2.p, 'm'))
          - 1000 * startValue(h3_great_circle_distance(t1.p, t2.p, 'km')))
        > 1e-3 * startValue(h3_great_circle_distance(t1.p, t2.p, 'm'));

-------------------------------------------------------------------------------

DROP TABLE tbl_th3index_known;
DROP TABLE tbl_tgeogpoint_known;

-------------------------------------------------------------------------------
