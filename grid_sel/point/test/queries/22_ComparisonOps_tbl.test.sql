-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint WHERE g #= temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogpoint WHERE g #= temp IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #= t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp #= t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint3D, tbl_geompoint3D WHERE g #= temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geogpoint3D WHERE g #= temp IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #= t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE t1.temp #= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint WHERE g #<> temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogpoint WHERE g #<> temp IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #<> t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint3D, tbl_geompoint3D WHERE g #<> temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geogpoint3D WHERE g #<> temp IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #<> t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
