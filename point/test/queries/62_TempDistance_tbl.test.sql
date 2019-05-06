-------------------------------------------------------------------------------

SELECT g <-> temp FROM tbl_geompoint, tbl_tgeompoint
WHERE g <-> temp IS NOT NULL LIMIT 10;
SELECT temp <-> g FROM tbl_tgeompoint, tbl_geompoint
WHERE temp <-> g IS NOT NULL LIMIT 10;
SELECT t1.temp <-> t2.temp FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp <-> t2.temp IS NOT NULL LIMIT 10;

SELECT g <-> temp FROM tbl_geogpoint, tbl_tgeogpoint
WHERE g <-> temp IS NOT NULL LIMIT 10;
SELECT temp <-> g FROM tbl_tgeogpoint, tbl_geogpoint
WHERE temp <-> g IS NOT NULL LIMIT 10;
SELECT t1.temp <-> t2.temp FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp <-> t2.temp IS NOT NULL LIMIT 10;

-------------------------------------------------------------------------------

SELECT g <-> temp FROM tbl_geompoint3D, tbl_tgeompoint3D
WHERE g <-> temp IS NOT NULL LIMIT 10;
SELECT temp <-> g FROM tbl_tgeompoint3D, tbl_geompoint3D
WHERE temp <-> g IS NOT NULL LIMIT 10;
SELECT t1.temp <-> t2.temp FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp <-> t2.temp IS NOT NULL LIMIT 10;

SELECT g <-> temp FROM tbl_geogpoint3D, tbl_tgeogpoint3D
WHERE g <-> temp IS NOT NULL LIMIT 10; 
SELECT temp <-> g FROM tbl_tgeogpoint3D, tbl_geogpoint3D
WHERE temp <-> g IS NOT NULL LIMIT 10;
SELECT t1.temp <-> t2.temp FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp <-> t2.temp IS NOT NULL LIMIT 10;

-------------------------------------------------------------------------------

	

