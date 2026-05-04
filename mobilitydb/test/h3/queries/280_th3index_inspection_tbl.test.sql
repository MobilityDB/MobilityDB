-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- Regression tests for §1.2 inspection — one scalar per query.
--
-------------------------------------------------------------------------------

-- Fixture: six known-valid h3 cells covering several resolutions.
-- Held as instants; sequence-form coverage is built per-query.

DROP TABLE IF EXISTS tbl_th3index_known;
CREATE TABLE tbl_th3index_known(k int PRIMARY KEY, temp th3index);
INSERT INTO tbl_th3index_known VALUES
  (1, th3index '590464338553208831@2001-01-01'),  -- res 3 hexagon
  (2, th3index '590464201114255359@2001-01-01'),  -- res 3 pentagon
  (3, th3index '595812165542215679@2001-01-01'),  -- res 4 pentagon
  (4, th3index '612544986753269759@2001-01-01'),  -- res 8 hexagon
  (5, th3index '612544986761658367@2001-01-01'),  -- res 8 hexagon (neighbour)
  (6, th3index '622236750694711295@2001-01-01');  -- res 10 NYC hexagon

-------------------------------------------------------------------------------
-- h3_get_resolution
-------------------------------------------------------------------------------

-- Sum of resolutions = 3+3+4+8+8+10 = 36
SELECT SUM(startValue(h3_get_resolution(temp)))::int FROM tbl_th3index_known;

-- All rows have a non-null resolution
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE h3_get_resolution(temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- h3_get_base_cell_number
-------------------------------------------------------------------------------

-- All base-cell numbers fall in [0, 121] (h3 has 122 base cells).
SELECT bool_and(startValue(h3_get_base_cell_number(temp)) BETWEEN 0 AND 121)
  FROM tbl_th3index_known;

-------------------------------------------------------------------------------
-- h3_is_valid_cell
-------------------------------------------------------------------------------

-- Every fixture row is a valid h3 cell.
SELECT bool_and(startValue(h3_is_valid_cell(temp))) FROM tbl_th3index_known;

-- Mixed sequence: a valid cell flanked by an invalid one.
SELECT startValue(h3_is_valid_cell(th3index
  '{590464338553208831@2001-01-01, 0@2001-01-02}'));

-------------------------------------------------------------------------------
-- h3_is_res_class_iii
-------------------------------------------------------------------------------

-- Class III iff resolution is odd. Fixture has resolutions 3, 3, 4, 8, 8, 10
-- → 2 class-III rows.
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE startValue(h3_is_res_class_iii(temp));

-------------------------------------------------------------------------------
-- h3_is_pentagon
-------------------------------------------------------------------------------

-- Fixture has 3 pentagons (rows 2, 3) and 4 hexagons.
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE startValue(h3_is_pentagon(temp));

-------------------------------------------------------------------------------

DROP TABLE tbl_th3index_known;

-------------------------------------------------------------------------------
