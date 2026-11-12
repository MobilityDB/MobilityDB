-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE g #= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography, tbl_geography WHERE g #= temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE t1.temp #= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2 WHERE t1.temp #= t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_geometry3D WHERE g #= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geography3D WHERE g #= temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp #= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp #= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE g #<> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography, tbl_geography WHERE g #<> temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE t1.temp #<> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_geometry3D WHERE g #<> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geography3D WHERE g #<> temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp #<> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

-- Ever and always overlaps: a temporal geometry lies in a box when its
-- restriction to the box is not empty, and always lies in it when its
-- restriction to the complement of the box is empty

SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE t1.temp ?&& stbox(t2.temp) <> (atStbox(t1.temp, stbox(t2.temp)) IS NOT NULL);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE t1.temp %&& stbox(t2.temp) <> (minusStbox(t1.temp, stbox(t2.temp)) IS NULL);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE t1.temp ?&& stbox(t2.temp);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE t1.temp %&& stbox(t2.temp);

-- Two temporal geometries that ever intersect ever have overlapping boxes
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE eIntersects(t1.temp, t2.temp) AND NOT t1.temp ?&& t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE t1.temp ?&& t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE t1.temp %&& t2.temp;

-------------------------------------------------------------------------------
