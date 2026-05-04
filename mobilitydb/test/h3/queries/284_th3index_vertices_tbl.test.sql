-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- Regression tests for §1.7 vertices — one scalar per query.
--
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_th3index_known;
CREATE TABLE tbl_th3index_known(k int PRIMARY KEY, temp th3index);
INSERT INTO tbl_th3index_known VALUES
  (1, th3index '590464338553208831@2001-01-01'),  -- res 3 hex   (6 vertices)
  (2, th3index '590464201114255359@2001-01-01'),  -- res 3 pent  (5 vertices)
  (3, th3index '595812165542215679@2001-01-01'),  -- res 4 pent  (5 vertices)
  (4, th3index '612544986753269759@2001-01-01'),  -- res 8 hex   (6 vertices)
  (5, th3index '612544986761658367@2001-01-01'),  -- res 8 hex   (6 vertices)
  (6, th3index '622236750694711295@2001-01-01');  -- res 10 hex  (6 vertices)

-------------------------------------------------------------------------------
-- h3_cell_to_vertex(th3index, integer)
-------------------------------------------------------------------------------

-- Vertex 0 exists for every fixture row (both hex and pent have ≥ 5 vertices).
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE h3_cell_to_vertex(temp, 0) IS NOT NULL;

-- Vertex 4 exists for both hex and pent.
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE h3_cell_to_vertex(temp, 4) IS NOT NULL;

-- For each cell, vertices 0..4 are pairwise distinct (at least 5 vertices).
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE h3_cell_to_vertex(temp, 0) = h3_cell_to_vertex(temp, 1)
     OR h3_cell_to_vertex(temp, 1) = h3_cell_to_vertex(temp, 2)
     OR h3_cell_to_vertex(temp, 2) = h3_cell_to_vertex(temp, 3)
     OR h3_cell_to_vertex(temp, 3) = h3_cell_to_vertex(temp, 4);

-------------------------------------------------------------------------------
-- h3_is_valid_vertex
-------------------------------------------------------------------------------

-- A freshly built vertex is valid for every fixture row.
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE NOT startValue(h3_is_valid_vertex(h3_cell_to_vertex(temp, 0)));

-- A plain cell is not a valid vertex.
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE startValue(h3_is_valid_vertex(temp));

-------------------------------------------------------------------------------
-- h3_vertex_to_latlng — needs h3_adapter.c body
-------------------------------------------------------------------------------

-- Every vertex maps to a non-null geodetic point.
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE h3_vertex_to_latlng(h3_cell_to_vertex(temp, 0)) IS NULL;

-------------------------------------------------------------------------------

DROP TABLE tbl_th3index_known;

-------------------------------------------------------------------------------
