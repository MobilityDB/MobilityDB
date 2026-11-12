-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Position operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp << t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp >> t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp &< t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp &> t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp <<| t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp |>> t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp <<# t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp #>> t2.temp;

-- Strictly before excludes overlapping
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2
WHERE t1.temp << t2.temp AND t1.temp && t2.temp;

-------------------------------------------------------------------------------
