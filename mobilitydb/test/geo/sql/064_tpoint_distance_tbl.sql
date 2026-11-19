SELECT round(MAX(maxValue(g <-> temp)), 6) FROM tbl_geom_point t1, tbl_tgeompoint t2
WHERE g <-> temp IS NOT NULL;
SELECT round(MAX(maxValue(temp <-> g)), 6) FROM tbl_tgeompoint t1, tbl_geom_point t2
WHERE temp <-> g IS NOT NULL;
SELECT round(MAX(maxValue(t1.temp <-> t2.temp)), 6) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp <-> t2.temp IS NOT NULL;

SELECT round(MAX(maxValue(g <-> temp)), 6) FROM tbl_geog_point t1, tbl_tgeogpoint t2
WHERE g <-> temp IS NOT NULL;
SELECT round(MAX(maxValue(temp <-> g)), 6) FROM tbl_tgeogpoint t1, tbl_geog_point t2
WHERE temp <-> g IS NOT NULL;
SELECT round(MAX(maxValue(t1.temp <-> t2.temp)), 6) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp <-> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT round(MAX(maxValue(g <-> temp)), 6) FROM tbl_geom_point3d t1, tbl_tgeompoint3d t2
WHERE g <-> temp IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(MAX(maxValue(temp <-> g)), 6) FROM tbl_tgeompoint3d t1, tbl_geom_point3d t2
WHERE temp <-> g IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(MAX(maxValue(t1.temp <-> t2.temp)), 6) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY 1 LIMIT 10;

SELECT round(MAX(maxValue(g <-> temp)), 6) FROM tbl_geog_point3d t1, tbl_tgeogpoint3d t2
WHERE g <-> temp IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(MAX(maxValue(temp <-> g)), 6) FROM tbl_tgeogpoint3d t1, tbl_geog_point3d t2
WHERE temp <-> g IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(MAX(maxValue(t1.temp <-> t2.temp)), 6) FROM tbl_tgeogpoint3d t1, tbl_tgeogpoint3d t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY 1 LIMIT 10;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom t
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_geom3d
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geog t
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_geog3d
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3d t1, tbl_tgeogpoint3d t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_geom3d t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geog t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_geog3d t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3d t1, tbl_tgeogpoint3d t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_geom3d t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geog t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_geog3d t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3d t1, tbl_tgeogpoint3d t2
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_geom3d t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geog t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
-- SELECT COUNT(*) FROM tbl_tgeogpoint3d,
-- ( SELECT * FROM tbl_geog3d LIMIT 10 ) t
-- WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3d t1, tbl_tgeogpoint3d t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

--------------------------------------------------------


-------------------------------------------------------------------------------
