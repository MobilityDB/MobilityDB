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

-------------------------------------------------------------------------------
-- Ever and always overlaps with a box: a cell ever lies in a box where its
-- restriction to the box is not empty, and always where its restriction to
-- the complement is empty
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp ?&& expandSpace(stbox(t2.temp), 1) <> (atStbox(t1.temp, expandSpace(stbox(t2.temp), 1)) IS NOT NULL);
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp %&& expandSpace(stbox(t2.temp), 1) <> (minusStbox(t1.temp, expandSpace(stbox(t2.temp), 1)) IS NULL);
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.k <= 30 AND t2.k <= 30
AND expandSpace(stbox(t2.temp), 1) ?&& t1.temp <> t1.temp ?&& expandSpace(stbox(t2.temp), 1);
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp ?&& expandSpace(stbox(t2.temp), 1);
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp %&& expandSpace(stbox(t2.temp), 1);
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp && expandSpace(stbox(t2.temp), 1);

-------------------------------------------------------------------------------
