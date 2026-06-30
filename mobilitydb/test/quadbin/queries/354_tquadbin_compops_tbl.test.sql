-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- Table-based tests for the tquadbin comparison operators.

-------------------------------------------------------------------------------
-- Ever / always equal and not equal
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_quadbin t1, tbl_tquadbin t2 WHERE eEq(t1.qb, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_quadbin t2 WHERE eEq(t1.temp, t2.qb) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE eEq(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_quadbin t1, tbl_tquadbin t2 WHERE aEq(t1.qb, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE eNe(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE aNe(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal equal and not equal
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_quadbin t1, tbl_tquadbin t2 WHERE tEq(t1.qb, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_quadbin t2 WHERE tEq(t1.temp, t2.qb) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE tEq(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE tNe(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
