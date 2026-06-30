-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- Table-based tests for the tquadbin type, exercised over tbl_tquadbin.

-------------------------------------------------------------------------------
-- Send / receive
-------------------------------------------------------------------------------

COPY tbl_tquadbin TO '/tmp/tbl_tquadbin' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tquadbin_tmp;
CREATE TABLE tbl_tquadbin_tmp AS TABLE tbl_tquadbin WITH NO DATA;
COPY tbl_tquadbin_tmp FROM '/tmp/tbl_tquadbin' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
DROP TABLE tbl_tquadbin_tmp;

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT SUM(numInstants(temp)) FROM tbl_tquadbin;
SELECT COUNT(*) FROM tbl_tquadbin WHERE startValue(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin WHERE endValue(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin WHERE getTime(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin WHERE duration(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin WHERE startInstant(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin WHERE endInstant(temp) IS NOT NULL;
SELECT SUM(numTimestamps(temp)) FROM tbl_tquadbin;
SELECT COUNT(*) FROM tbl_tquadbin WHERE memSize(temp) > 0;

-------------------------------------------------------------------------------
-- Restrictions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tquadbin WHERE atTime(temp, getTime(temp)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin WHERE minusTime(temp, getTime(temp)) IS NULL;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp <> t2.temp;

-------------------------------------------------------------------------------
