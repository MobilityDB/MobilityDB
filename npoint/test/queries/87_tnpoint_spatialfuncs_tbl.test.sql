-------------------------------------------------------------------------------

SELECT MAX(st_npoints(trajectory(temp))) FROM tbl_tnpoint;

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE atGeometry(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE minusGeometry(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL;

SELECT round(MAX(length(temp))::numeric, 6) FROM tbl_tnpoint;

SELECT round(MAX(maxValue(cumulativeLength(temp)))::numeric, 6) FROM tbl_tnpoint;

SELECT round(MAX(maxValue(speed(temp)))::numeric, 6) FROM tbl_tnpoint;

SELECT round(azimuth(temp), 13) FROM tbl_tnpoint WHERE azimuth(temp) IS NOT NULL LIMIT 10;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE nearestApproachInstant(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE nearestApproachInstant(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE nearestApproachInstant(t1.temp, t2.np) IS NOT NULL;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE nearestApproachInstant(t1.np, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE nearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE nearestApproachDistance(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE nearestApproachDistance(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE nearestApproachDistance(t1.temp, t2.np) IS NOT NULL;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE nearestApproachDistance(t1.np, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE nearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE shortestLine(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE shortestLine(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE shortestLine(t1.temp, t2.np) IS NOT NULL;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE shortestLine(t1.np, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE shortestLine(t1.np, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;


-------------------------------------------------------------------------------

