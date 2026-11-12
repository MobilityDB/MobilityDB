-- Table-based tests for the th3index comparison operators.

-------------------------------------------------------------------------------
-- Ever / always equal and not equal
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_h3index t1, tbl_th3index t2 WHERE eEqual(t1.h3, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_h3index t2 WHERE eEqual(t1.temp, t2.h3) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE eEqual(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_h3index t1, tbl_th3index t2 WHERE aEqual(t1.h3, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE eNotEqual(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE aNotEqual(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal equal and not equal
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_h3index t1, tbl_th3index t2 WHERE tEqual(t1.h3, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_h3index t2 WHERE tEqual(t1.temp, t2.h3) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE tEqual(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE tNotEqual(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
