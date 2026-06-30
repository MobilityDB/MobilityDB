-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- Table-based tests for the th3index comparison operators.

-------------------------------------------------------------------------------
-- Ever / always equal and not equal
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_h3index t1, tbl_th3index t2 WHERE eEq(t1.h3, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_h3index t2 WHERE eEq(t1.temp, t2.h3) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE eEq(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_h3index t1, tbl_th3index t2 WHERE aEq(t1.h3, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE eNe(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE aNe(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal equal and not equal
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_h3index t1, tbl_th3index t2 WHERE tEq(t1.h3, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_h3index t2 WHERE tEq(t1.temp, t2.h3) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE tEq(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE tNe(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
