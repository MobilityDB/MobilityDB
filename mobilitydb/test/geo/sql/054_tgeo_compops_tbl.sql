-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE g #= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography, tbl_geography WHERE g #= temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE t1.temp #= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2 WHERE t1.temp #= t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry3d, tbl_geometry3d WHERE g #= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3d, tbl_geography3d WHERE g #= temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry3d t1, tbl_tgeometry3d t2 WHERE t1.temp #= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3d t1, tbl_tgeography3d t2 WHERE t1.temp #= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE g #<> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography, tbl_geography WHERE g #<> temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE t1.temp #<> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry3d, tbl_geometry3d WHERE g #<> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3d, tbl_geography3d WHERE g #<> temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry3d t1, tbl_tgeometry3d t2 WHERE t1.temp #<> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3d t1, tbl_tgeography3d t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

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

-- A temporal geography is read against the bounds of a geodetic box, the
-- region of its longitudes and latitudes, its edges being great circles
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2 WHERE t1.temp ?&& stbox(t2.temp);
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2 WHERE t1.temp %&& stbox(t2.temp);
-- The great circle from Point(10 60) to Point(50 60) rises to latitude 61.52
SELECT tgeography '[Linestring(10 60,50 60)@2001-01-01, Linestring(10 60,50 60)@2001-01-02]'
  %&& stbox 'SRID=4326;GEODSTBOX X((10,55),(50,61))';
SELECT tgeography '[Linestring(10 60,50 60)@2001-01-01, Linestring(10 60,50 60)@2001-01-02]'
  %&& stbox 'SRID=4326;GEODSTBOX X((10,55),(50,62))';
-- The edge crosses a box holding none of its vertices
SELECT tgeography '[Linestring(10 60,50 60)@2001-01-01, Linestring(10 60,50 60)@2001-01-02]'
  ?&& stbox 'SRID=4326;GEODSTBOX X((25,61),(35,62))';
SELECT tgeography '[Linestring(10 60,50 60)@2001-01-01, Linestring(10 60,50 60)@2001-01-02]'
  ?&& stbox 'SRID=4326;GEODSTBOX X((25,62),(35,63))';
-- A polygon holds a box which none of its vertices and edges reaches
SELECT tgeography 'Polygon((0 0,20 0,20 20,0 20,0 0))@2001-01-01'
  ?&& stbox 'SRID=4326;GEODSTBOX X((9,9),(11,11))';
SELECT tgeography 'Polygon((0 0,20 0,20 20,0 20,0 0))@2001-01-01'
  %&& stbox 'SRID=4326;GEODSTBOX X((9,9),(11,11))';

-- Two temporal geometries that ever intersect ever have overlapping boxes
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE eIntersects(t1.temp, t2.temp) AND NOT t1.temp ?&& t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE t1.temp ?&& t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE t1.temp %&& t2.temp;

-------------------------------------------------------------------------------
