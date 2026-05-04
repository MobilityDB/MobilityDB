-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- Regression tests for §1.10 casts — one scalar per query.
--
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_th3index_known;
CREATE TABLE tbl_th3index_known(k int PRIMARY KEY, temp th3index);
INSERT INTO tbl_th3index_known VALUES
  (1, th3index '590464338553208831@2001-01-01'),
  (2, th3index '590464201114255359@2001-01-01'),
  (3, th3index '595812165542215679@2001-01-01'),
  (4, th3index '612544986753269759@2001-01-01'),
  (5, th3index '612544986761658367@2001-01-01'),
  (6, th3index '622236750694711295@2001-01-01');

-------------------------------------------------------------------------------
-- th3index :: tgeogpoint
-------------------------------------------------------------------------------

-- All rows cast to a non-null tgeogpoint.
SELECT COUNT(*) FROM tbl_th3index_known WHERE temp::tgeogpoint IS NULL;

-- Cast is equivalent to h3_cell_to_latlng (mismatches counted).
SELECT COUNT(*) FROM tbl_th3index_known
  WHERE NOT (temp::tgeogpoint ~= h3_cell_to_latlng(temp));

-------------------------------------------------------------------------------
-- th3index :: tgeompoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_th3index_known WHERE temp::tgeompoint IS NULL;

SELECT COUNT(*) FROM tbl_th3index_known
  WHERE NOT (temp::tgeompoint ~= h3_cell_to_latlng_tgeompoint(temp));

-------------------------------------------------------------------------------

DROP TABLE tbl_th3index_known;

-------------------------------------------------------------------------------
