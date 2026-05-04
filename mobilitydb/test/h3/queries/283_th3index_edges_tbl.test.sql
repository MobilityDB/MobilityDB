-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- Regression tests for §1.6 directed edges — one scalar per query.
--
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_th3index_known;
CREATE TABLE tbl_th3index_known(k int PRIMARY KEY, temp th3index);
INSERT INTO tbl_th3index_known VALUES
  (1, th3index '590464338553208831@2001-01-01'),  -- res 3 hex
  (2, th3index '590464201114255359@2001-01-01'),  -- res 3 pentagon
  (3, th3index '595812165542215679@2001-01-01'),  -- res 4 pentagon
  (4, th3index '612544986753269759@2001-01-01'),  -- res 8 hex
  (5, th3index '612544986761658367@2001-01-01'),  -- res 8 hex (neighbour of #4)
  (6, th3index '622236750694711295@2001-01-01');  -- res 10 NYC hex

-- Pre-compute the cross product for binary tests
DROP TABLE IF EXISTS tbl_th3index_pairs;
CREATE TABLE tbl_th3index_pairs AS
  SELECT t1.k AS k1, t2.k AS k2, t1.temp AS a, t2.temp AS b
  FROM tbl_th3index_known t1, tbl_th3index_known t2;
ANALYZE tbl_th3index_pairs;

-------------------------------------------------------------------------------
-- h3_are_neighbor_cells
-------------------------------------------------------------------------------

-- A cell is never its own neighbour
SELECT COUNT(*) FROM tbl_th3index_pairs
  WHERE k1 = k2 AND startValue(h3_are_neighbor_cells(a, b));

-- The (#4, #5) pair is the only known neighbouring pair in the fixture.
-- Symmetric: count of unordered pairs with neighbour=true is 2 (forward + reverse).
SELECT COUNT(*) FROM tbl_th3index_pairs
  WHERE k1 <> k2 AND startValue(h3_are_neighbor_cells(a, b));

-------------------------------------------------------------------------------
-- h3_cells_to_directed_edge — round trip via origin/destination accessors
-------------------------------------------------------------------------------

-- Origin of edge(o, d) = o, for the known neighbouring pair only.
SELECT COUNT(*) FROM tbl_th3index_pairs
  WHERE k1 = 4 AND k2 = 5
    AND h3_get_directed_edge_origin(h3_cells_to_directed_edge(a, b)) <> a;

-- Destination of edge(o, d) = d, for the known neighbouring pair only.
SELECT COUNT(*) FROM tbl_th3index_pairs
  WHERE k1 = 4 AND k2 = 5
    AND h3_get_directed_edge_destination(h3_cells_to_directed_edge(a, b)) <> b;

-------------------------------------------------------------------------------
-- h3_is_valid_directed_edge
-------------------------------------------------------------------------------

-- Plain cells are not valid directed edges.
SELECT bool_and(NOT startValue(h3_is_valid_directed_edge(temp)))
  FROM tbl_th3index_known;

-- A built edge is a valid directed edge.
SELECT startValue(h3_is_valid_directed_edge(
  h3_cells_to_directed_edge(
    th3index '612544986753269759@2001-01-01',
    th3index '612544986761658367@2001-01-01')));

-------------------------------------------------------------------------------
-- h3_get_directed_edge_origin / destination — non-null for valid edges
-------------------------------------------------------------------------------

SELECT bool_and(
    h3_get_directed_edge_origin(h3_cells_to_directed_edge(a, b)) IS NOT NULL)
  FROM tbl_th3index_pairs WHERE (k1, k2) IN ((4, 5), (5, 4));

SELECT bool_and(
    h3_get_directed_edge_destination(h3_cells_to_directed_edge(a, b)) IS NOT NULL)
  FROM tbl_th3index_pairs WHERE (k1, k2) IN ((4, 5), (5, 4));

-------------------------------------------------------------------------------
-- h3_directed_edge_to_boundary — needs h3_adapter.c body
-------------------------------------------------------------------------------

SELECT bool_and(h3_directed_edge_to_boundary(
    h3_cells_to_directed_edge(a, b)) IS NOT NULL)
  FROM tbl_th3index_pairs WHERE (k1, k2) IN ((4, 5), (5, 4));

-------------------------------------------------------------------------------

DROP TABLE tbl_th3index_pairs;
DROP TABLE tbl_th3index_known;

-------------------------------------------------------------------------------
