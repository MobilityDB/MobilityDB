-- Table-based tests for the s2cellset type, exercised over tbl_s2cellset.

-------------------------------------------------------------------------------
-- Send / receive
-------------------------------------------------------------------------------

\copy tbl_s2cellset TO 'tbl_s2cellset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_s2cellset_tmp;
CREATE TABLE tbl_s2cellset_tmp AS TABLE tbl_s2cellset WITH NO DATA;
\copy tbl_s2cellset_tmp FROM 'tbl_s2cellset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_s2cellset t1, tbl_s2cellset_tmp t2 WHERE t1.k = t2.k AND t1.s <> t2.s;
DROP TABLE tbl_s2cellset_tmp;

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT SUM(numValues(s)) FROM tbl_s2cellset;
SELECT COUNT(*) FROM tbl_s2cellset WHERE memSize(s) IS NOT NULL;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_s2cellset t1, tbl_s2cellset t2 WHERE t1.s = t2.s;
SELECT COUNT(*) FROM tbl_s2cellset t1, tbl_s2cellset t2 WHERE t1.s <> t2.s;

SELECT MAX(hash(s)) FROM tbl_s2cellset;

SELECT MAX(hashExtended(s, 1)) FROM tbl_s2cellset;

-------------------------------------------------------------------------------
