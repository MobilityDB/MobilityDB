-------------------------------------------------------------------------------

SELECT round(g <-> temp, 6) FROM tbl_geompoint t1, tbl_tgeompoint t2
WHERE g <-> temp IS NOT NULL ORDER BY t1.k LIMIT 10;
SELECT round(temp <-> g, 6) FROM tbl_tgeompoint t1, tbl_geompoint t2
WHERE temp <-> g IS NOT NULL ORDER BY t1.k LIMIT 10;
SELECT round(t1.temp <-> t2.temp, 6) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY t1.k LIMIT 10;

SELECT round(round(g <-> temp, 6), 6) FROM tbl_geogpoint t1, tbl_tgeogpoint t2
WHERE g <-> temp IS NOT NULL ORDER BY t1.k LIMIT 10;
SELECT round(round(temp <-> g, 6), 6) FROM tbl_tgeogpoint t1, tbl_geogpoint t2
WHERE temp <-> g IS NOT NULL ORDER BY t1.k LIMIT 10;
SELECT round(round(t1.temp <-> t2.temp, 6), 6) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY t1.k LIMIT 10;

-------------------------------------------------------------------------------

SELECT round(g <-> temp, 6) FROM tbl_geompoint3D t1, tbl_tgeompoint3D t2
WHERE g <-> temp IS NOT NULL ORDER BY t1.k LIMIT 10;
SELECT round(temp <-> g, 6) FROM tbl_tgeompoint3D t1, tbl_geompoint3D t2
WHERE temp <-> g IS NOT NULL ORDER BY t1.k LIMIT 10;
SELECT round(t1.temp <-> t2.temp, 6) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY t1.k LIMIT 10;

SELECT round(round(g <-> temp, 6), 6) FROM tbl_geogpoint3D t1, tbl_tgeogpoint3D t2
WHERE g <-> temp IS NOT NULL ORDER BY t1.k LIMIT 10; 
SELECT round(round(temp <-> g, 6), 6) FROM tbl_tgeogpoint3D t1, tbl_geogpoint3D t2
WHERE temp <-> g IS NOT NULL ORDER BY t1.k LIMIT 10;
SELECT round(round(t1.temp <-> t2.temp, 6), 6) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY t1.k LIMIT 10;

-------------------------------------------------------------------------------

	

