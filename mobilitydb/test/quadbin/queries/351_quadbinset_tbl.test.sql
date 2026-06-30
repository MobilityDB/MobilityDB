-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- Table-based tests for the quadbinset type, exercised over tbl_quadbinset.

-------------------------------------------------------------------------------
-- Send / receive
-------------------------------------------------------------------------------

COPY tbl_quadbinset TO '/tmp/tbl_quadbinset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_quadbinset_tmp;
CREATE TABLE tbl_quadbinset_tmp AS TABLE tbl_quadbinset WITH NO DATA;
COPY tbl_quadbinset_tmp FROM '/tmp/tbl_quadbinset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_quadbinset t1, tbl_quadbinset_tmp t2 WHERE t1.k = t2.k AND t1.s <> t2.s;
DROP TABLE tbl_quadbinset_tmp;

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT SUM(numValues(s)) FROM tbl_quadbinset;
SELECT COUNT(*) FROM tbl_quadbinset WHERE memSize(s) IS NOT NULL;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_quadbinset t1, tbl_quadbinset t2 WHERE t1.s = t2.s;
SELECT COUNT(*) FROM tbl_quadbinset t1, tbl_quadbinset t2 WHERE t1.s <> t2.s;

-------------------------------------------------------------------------------
