-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- Table-based tests for the h3indexset type, exercised over tbl_h3indexset.

-------------------------------------------------------------------------------
-- Send / receive
-------------------------------------------------------------------------------

COPY tbl_h3indexset TO '/tmp/tbl_h3indexset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_h3indexset_tmp;
CREATE TABLE tbl_h3indexset_tmp AS TABLE tbl_h3indexset WITH NO DATA;
COPY tbl_h3indexset_tmp FROM '/tmp/tbl_h3indexset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_h3indexset t1, tbl_h3indexset_tmp t2 WHERE t1.k = t2.k AND t1.s <> t2.s;
DROP TABLE tbl_h3indexset_tmp;

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT SUM(numValues(s)) FROM tbl_h3indexset;
SELECT COUNT(*) FROM tbl_h3indexset WHERE memSize(s) IS NOT NULL;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_h3indexset t1, tbl_h3indexset t2 WHERE t1.s = t2.s;
SELECT COUNT(*) FROM tbl_h3indexset t1, tbl_h3indexset t2 WHERE t1.s <> t2.s;

-------------------------------------------------------------------------------
