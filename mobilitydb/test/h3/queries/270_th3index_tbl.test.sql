-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- Table-based tests for the th3index type, exercised over tbl_th3index.

-------------------------------------------------------------------------------
-- Send / receive
-------------------------------------------------------------------------------

COPY tbl_th3index TO '/tmp/tbl_th3index' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_th3index_tmp;
CREATE TABLE tbl_th3index_tmp AS TABLE tbl_th3index WITH NO DATA;
COPY tbl_th3index_tmp FROM '/tmp/tbl_th3index' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
DROP TABLE tbl_th3index_tmp;

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT SUM(numInstants(temp)) FROM tbl_th3index;
SELECT COUNT(*) FROM tbl_th3index WHERE startValue(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index WHERE endValue(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index WHERE getTime(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index WHERE duration(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index WHERE startInstant(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index WHERE endInstant(temp) IS NOT NULL;
SELECT SUM(numTimestamps(temp)) FROM tbl_th3index;
SELECT COUNT(*) FROM tbl_th3index WHERE memSize(temp) > 0;

-------------------------------------------------------------------------------
-- Restrictions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_th3index WHERE atTime(temp, getTime(temp)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index WHERE minusTime(temp, getTime(temp)) IS NULL;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp <> t2.temp;

-------------------------------------------------------------------------------
