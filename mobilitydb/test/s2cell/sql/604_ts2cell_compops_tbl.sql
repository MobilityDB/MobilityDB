-- Table-based tests for the ts2cell comparison operators.

-------------------------------------------------------------------------------
-- Ever / always equal and not equal
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_s2cell t1, tbl_ts2cell t2 WHERE eEqual(t1.cell, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_s2cell t2 WHERE eEqual(t1.temp, t2.cell) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE eEqual(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_s2cell t1, tbl_ts2cell t2 WHERE aEqual(t1.cell, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE eNotEqual(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE aNotEqual(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal equal and not equal
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_s2cell t1, tbl_ts2cell t2 WHERE tEqual(t1.cell, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_s2cell t2 WHERE tEqual(t1.temp, t2.cell) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE tEqual(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE tNotEqual(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
