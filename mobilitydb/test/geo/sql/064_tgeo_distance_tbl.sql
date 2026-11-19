-------------------------------------------------------------------------------

SELECT round(MAX(maxValue(g <-> temp)), 6) FROM tbl_geometry t1, tbl_tgeometry t2
WHERE g <-> temp IS NOT NULL;
SELECT round(MAX(maxValue(temp <-> g)), 6) FROM tbl_tgeometry t1, tbl_geometry t2
WHERE temp <-> g IS NOT NULL;
SELECT round(MAX(maxValue(t1.temp <-> t2.temp)), 6) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE t1.temp <-> t2.temp IS NOT NULL;

SELECT round(MAX(maxValue(g <-> temp)), 6) FROM tbl_geography t1, tbl_tgeography t2
WHERE g <-> temp IS NOT NULL;
SELECT round(MAX(maxValue(temp <-> g)), 6) FROM tbl_tgeography t1, tbl_geography t2
WHERE temp <-> g IS NOT NULL;
SELECT round(MAX(maxValue(t1.temp <-> t2.temp)), 6) FROM tbl_tgeography t1, tbl_tgeography t2
WHERE t1.temp <-> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT round(MAX(maxValue(g <-> temp)), 6) FROM tbl_geometry3d t1, tbl_tgeometry3d t2
WHERE g <-> temp IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(MAX(maxValue(temp <-> g)), 6) FROM tbl_tgeometry3d t1, tbl_geometry3d t2
WHERE temp <-> g IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(MAX(maxValue(t1.temp <-> t2.temp)), 6) FROM tbl_tgeometry3d t1, tbl_tgeometry3d t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY 1 LIMIT 10;

SELECT round(MAX(maxValue(g <-> temp)), 6) FROM tbl_geography3d t1, tbl_tgeography3d t2
WHERE g <-> temp IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(MAX(maxValue(temp <-> g)), 6) FROM tbl_tgeography3d t1, tbl_geography3d t2
WHERE temp <-> g IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(MAX(maxValue(t1.temp <-> t2.temp)), 6) FROM tbl_tgeography3d t1, tbl_tgeography3d t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY 1 LIMIT 10;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry t
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3d, tbl_geometry3d
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3d t1, tbl_tgeometry3d t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeography, tbl_geography t
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3d, tbl_geography3d
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3d t1, tbl_tgeography3d t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3d, tbl_geometry3d t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3d t1, tbl_tgeometry3d t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeography, tbl_geography t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3d, tbl_geography3d t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3d t1, tbl_tgeography3d t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3d, tbl_geometry3d t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3d t1, tbl_tgeometry3d t2
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeography, tbl_geography t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2
WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3d, tbl_geography3d t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3d t1, tbl_tgeography3d t2
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3d, tbl_geometry3d t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3d t1, tbl_tgeometry3d t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeography, tbl_geography t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
-- SELECT COUNT(*) FROM tbl_tgeography3d,
-- ( SELECT * FROM tbl_geography3d LIMIT 10 ) t
-- WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3d t1, tbl_tgeography3d t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
