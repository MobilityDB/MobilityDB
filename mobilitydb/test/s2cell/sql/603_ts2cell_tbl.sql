-- Table-based tests for the ts2cell type, exercised over tbl_ts2cell.

-------------------------------------------------------------------------------
-- Send / receive
-------------------------------------------------------------------------------

\copy tbl_ts2cell TO 'tbl_ts2cell' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_ts2cell_tmp;
CREATE TABLE tbl_ts2cell_tmp AS TABLE tbl_ts2cell WITH NO DATA;
\copy tbl_ts2cell_tmp FROM 'tbl_ts2cell' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
DROP TABLE tbl_ts2cell_tmp;

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT SUM(numInstants(temp)) FROM tbl_ts2cell;
SELECT COUNT(*) FROM tbl_ts2cell WHERE startValue(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ts2cell WHERE endValue(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ts2cell WHERE getTime(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ts2cell WHERE duration(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ts2cell WHERE startInstant(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ts2cell WHERE endInstant(temp) IS NOT NULL;
SELECT SUM(numTimestamps(temp)) FROM tbl_ts2cell;
SELECT COUNT(*) FROM tbl_ts2cell WHERE memSize(temp) > 0;

-------------------------------------------------------------------------------
-- Restrictions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ts2cell WHERE atTime(temp, getTime(temp)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ts2cell WHERE minusTime(temp, getTime(temp)) IS NULL;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp <> t2.temp;

-------------------------------------------------------------------------------
