-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- Regression tests for §1.3 grid traversal — one scalar per query.
--
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_th3index_known;
CREATE TABLE tbl_th3index_known(k int PRIMARY KEY, temp th3index);
INSERT INTO tbl_th3index_known VALUES
  (1, th3index '612544986753269759@2001-01-01'),  -- res 8 hex
  (2, th3index '612544986761658367@2001-01-01'),  -- res 8 hex (neighbour of #1)
  (3, th3index '622236750694711295@2001-01-01');  -- res 10 NYC hex

DROP TABLE IF EXISTS tbl_th3index_pairs;
CREATE TABLE tbl_th3index_pairs AS
  SELECT t1.k AS k1, t2.k AS k2, t1.temp AS a, t2.temp AS b
  FROM tbl_th3index_known t1, tbl_th3index_known t2;

-------------------------------------------------------------------------------
-- h3_grid_distance
-------------------------------------------------------------------------------

-- Distance from any cell to itself is 0.
SELECT COUNT(*) FROM tbl_th3index_pairs
  WHERE k1 = k2 AND startValue(h3_grid_distance(a, b)) <> 0;

-- The known neighbouring pair is at distance 1.
SELECT startValue(h3_grid_distance(a, b))
  FROM tbl_th3index_pairs WHERE k1 = 1 AND k2 = 2;

-- Distance is symmetric.
SELECT COUNT(*) FROM tbl_th3index_pairs p1, tbl_th3index_pairs p2
  WHERE p1.k1 = p2.k2 AND p1.k2 = p2.k1
    AND startValue(h3_grid_distance(p1.a, p1.b))
        <> startValue(h3_grid_distance(p2.a, p2.b));

-------------------------------------------------------------------------------
-- The `<->` operator (synonym for h3_grid_distance per §6.1)
-------------------------------------------------------------------------------

-- Operator and function form must agree on every pair.
SELECT COUNT(*) FROM tbl_th3index_pairs
  WHERE startValue(a <-> b) <> startValue(h3_grid_distance(a, b));

-------------------------------------------------------------------------------
-- h3_cell_to_local_ij
-------------------------------------------------------------------------------

-- Every same-resolution intra-base-cell pair produces a non-null local IJ.
-- The (#1, #2) and (#2, #1) pairs are guaranteed to work; cross-resolution
-- pairs (#1, #3) etc may legitimately error in libh3 — restrict the sample.
SELECT COUNT(*) FROM tbl_th3index_pairs
  WHERE k1 IN (1, 2) AND k2 IN (1, 2)
    AND h3_cell_to_local_ij(a, b) IS NULL;

-------------------------------------------------------------------------------
-- h3_local_ij_to_cell — round trip
--
-- For the (origin, cell) pairs that map cleanly to local IJ,
-- local_ij_to_cell(origin, cell_to_local_ij(origin, c)) = c.
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_th3index_pairs
  WHERE k1 IN (1, 2) AND k2 IN (1, 2)
    AND h3_local_ij_to_cell(a, h3_cell_to_local_ij(a, b)) <> b;

-------------------------------------------------------------------------------

DROP TABLE tbl_th3index_pairs;
DROP TABLE tbl_th3index_known;

-------------------------------------------------------------------------------
