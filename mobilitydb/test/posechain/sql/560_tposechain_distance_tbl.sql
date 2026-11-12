-------------------------------------------------------------------------------
-- Table queries
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2
WHERE tDistance(t1.temp, t2.temp) IS NOT NULL LIMIT 10;

SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2
WHERE nearestApproachInstant(t1.temp, t2.temp) IS NOT NULL LIMIT 10;

SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2
WHERE nearestApproachDistance(t1.temp, t2.temp) IS NOT NULL LIMIT 10;

SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL LIMIT 10;

