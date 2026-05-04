-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- Regression tests for §1.4 hierarchy — one scalar per query.
--
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_th3index_known;
CREATE TABLE tbl_th3index_known(k int PRIMARY KEY, temp th3index);
INSERT INTO tbl_th3index_known VALUES
  (1, th3index '590464338553208831@2001-01-01'),  -- res 3 hexagon
  (2, th3index '590464201114255359@2001-01-01'),  -- res 3 pentagon
  (3, th3index '595812165542215679@2001-01-01'),  -- res 4 pentagon
  (4, th3index '612544986753269759@2001-01-01'),  -- res 8 hexagon
  (5, th3index '612544986761658367@2001-01-01'),  -- res 8 hexagon
  (6, th3index '622236750694711295@2001-01-01');  -- res 10 NYC hexagon

-------------------------------------------------------------------------------
-- h3_cell_to_parent(th3index, integer)
-------------------------------------------------------------------------------

-- For every row, the parent at resolution 0 has resolution 0.
SELECT bool_and(startValue(h3_get_resolution(h3_cell_to_parent(temp, 0))) = 0)
  FROM tbl_th3index_known;

-- Parent at resolution 2 has resolution 2 (only valid for cells with res >= 2,
-- which is all our fixture rows).
SELECT bool_and(startValue(h3_get_resolution(h3_cell_to_parent(temp, 2))) = 2)
  FROM tbl_th3index_known;

-------------------------------------------------------------------------------
-- h3_cell_to_parent(th3index)
-------------------------------------------------------------------------------

-- Each parent is exactly one resolution coarser.
SELECT bool_and(
    startValue(h3_get_resolution(h3_cell_to_parent(temp))) =
    startValue(h3_get_resolution(temp)) - 1)
  FROM tbl_th3index_known;

-- Idempotent: parent(parent(c)) = parent(c, res-2)
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE h3_cell_to_parent(h3_cell_to_parent(temp))
      <> h3_cell_to_parent(temp,
           startValue(h3_get_resolution(temp)) - 2);

-------------------------------------------------------------------------------
-- h3_cell_to_center_child(th3index, integer)
-------------------------------------------------------------------------------

-- center_child at resolution 12 has resolution 12 for all fixture rows.
SELECT bool_and(startValue(h3_get_resolution(
    h3_cell_to_center_child(temp, 12))) = 12)
  FROM tbl_th3index_known;

-- Round trip: parent(center_child(c, R)) = c when R > res(c).
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE h3_cell_to_parent(
          h3_cell_to_center_child(temp, 14),
          startValue(h3_get_resolution(temp))) <> temp;

-------------------------------------------------------------------------------
-- h3_cell_to_center_child(th3index)
-------------------------------------------------------------------------------

-- Each center_child is exactly one resolution finer (and < 15).
SELECT bool_and(
    startValue(h3_get_resolution(h3_cell_to_center_child(temp))) =
    startValue(h3_get_resolution(temp)) + 1)
  FROM tbl_th3index_known;

-------------------------------------------------------------------------------
-- h3_cell_to_child_pos(th3index, integer)
-------------------------------------------------------------------------------

-- For every row, position relative to the immediate parent is in [0, 6].
SELECT bool_and(startValue(h3_cell_to_child_pos(temp,
                  startValue(h3_get_resolution(temp)) - 1)) BETWEEN 0 AND 6)
  FROM tbl_th3index_known;

-------------------------------------------------------------------------------
-- h3_child_pos_to_cell(tbigint, th3index, integer)
--
-- Round-trip identity: child_pos_to_cell(cell_to_child_pos(c, R), parent, res(c))
-- = c, where parent = parent(c, R).
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_th3index_known
  WHERE h3_child_pos_to_cell(
          h3_cell_to_child_pos(temp,
            startValue(h3_get_resolution(temp)) - 1),
          h3_cell_to_parent(temp),
          startValue(h3_get_resolution(temp)))
        <> temp;

-------------------------------------------------------------------------------

DROP TABLE tbl_th3index_known;

-------------------------------------------------------------------------------
