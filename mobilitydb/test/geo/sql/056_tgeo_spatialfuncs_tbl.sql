-------------------------------------------------------------------------------

SELECT DISTINCT SRID(temp) FROM tbl_tgeometry;
SELECT DISTINCT SRID(temp) FROM tbl_tgeography;
SELECT DISTINCT SRID(temp) FROM tbl_tgeometry3D;
SELECT DISTINCT SRID(temp) FROM tbl_tgeography3D;

/*
SELECT setSRID(temp,3812) FROM tbl_tgeometry;
SELECT setSRID(temp,4326) FROM tbl_tgeography;
SELECT setSRID(temp,3812) FROM tbl_tgeometry3D;
SELECT setSRID(temp,4326) FROM tbl_tgeography3D;
*/

SELECT COUNT(*) FROM tbl_tgeometry WHERE startValue(transform(setSRID(temp, 3812), 4326)) = ST_Transform(ST_SetSRID(startValue(temp), 3812), 4326);
SELECT COUNT(*) FROM tbl_tgeometry3D WHERE startValue(transform(setSRID(temp, 3812), 4326)) = ST_Transform(ST_SetSRID(startValue(temp), 3812), 4326);

-------------------------------------------------------------------------------

SELECT tgeography(setSRID(temp,4326)) FROM tbl_tgeometry LIMIT 10;
SELECT tgeometry(temp) FROM tbl_tgeography LIMIT 10;
SELECT tgeography(setSRID(temp,7844)) FROM tbl_tgeometry3D LIMIT 10;
SELECT tgeometry(temp) FROM tbl_tgeography3D LIMIT 10;

SELECT setSRID(temp,4326)::tgeography FROM tbl_tgeometry LIMIT 10;
SELECT temp::tgeometry FROM tbl_tgeography LIMIT 10;
SELECT setSRID(temp,4326)::tgeography FROM tbl_tgeometry3D LIMIT 10;
SELECT temp::tgeometry FROM tbl_tgeography3D LIMIT 10;

SELECT asText(round(temp, 2)) FROM tbl_tgeometry LIMIT 10;
SELECT asText(round(temp, 2)) FROM tbl_tgeography LIMIT 10;
SELECT asText(round(temp, 2)) FROM tbl_tgeometry3D LIMIT 10;
SELECT asText(round(temp, 2)) FROM tbl_tgeography3D LIMIT 10;

-- Round an array of temporal points
SELECT asText(round(array_agg(inst ORDER BY k), 2)) FROM tbl_tgeometry_inst WHERE inst IS NOT NULL AND k % 20 = 1;
SELECT asText(round(array_agg(inst ORDER BY k), 2)) FROM tbl_tgeography_inst WHERE inst IS NOT NULL AND k % 20 = 1;

SELECT MAX(ST_NPoints(traversedArea(temp))) FROM tbl_tgeometry;
SELECT MAX(ST_NPoints(traversedArea(temp)::geometry)) FROM tbl_tgeography;
SELECT MAX(ST_NPoints(traversedArea(temp))) FROM tbl_tgeometry3D;
SELECT MAX(ST_NPoints(traversedArea(temp)::geometry)) FROM tbl_tgeography3D;

SELECT MIN(numInstants(centroid(temp))) FROM tbl_tgeometry;
SELECT MIN(numInstants(centroid(temp))) FROM tbl_tgeography;

-------------------------------------------------------------------------------

SELECT MAX(ST_Area(convexHull(temp))) FROM tbl_tgeometry;

-------------------------------------------------------------------------------
-- Verification that the SRID is kept during atGeometry/minusGeometry and
-- atStbox/minusStbox operations

SELECT DISTINCT SRID(temp), ST_SRID(g) FROM tbl_tgeometry t1, tbl_geometry t2;
SELECT DISTINCT SRID(atGeometry(temp, g)) FROM tbl_tgeometry t1, tbl_geometry t2;
SELECT DISTINCT SRID(minusGeometry(temp, g)) FROM tbl_tgeometry t1, tbl_geometry t2;

SELECT DISTINCT SRID(temp), SRID(setSRID(b, 3812)) FROM tbl_tgeometry t1, tbl_stbox t2;
SELECT DISTINCT SRID(atStbox(temp, setSRID(b, 3812))) FROM tbl_tgeometry t1, tbl_stbox t2;
SELECT DISTINCT SRID(minusStbox(temp, setSRID(b, 3812))) FROM tbl_tgeometry t1, tbl_stbox t2;

-------------------------------------------------------------------------------
-- Modulo used to reduce time needed for the tests
-- The restriction of a temporal geometry to a geometry clips each value to the
-- geometry and its complement clips it to the outside, so an instant whose
-- value the geometry splits belongs to both: together they cover the time of
-- the value, and their merge is the value only for a value never split
WITH r AS (SELECT temp, atGeometry(temp, g) AS a, minusGeometry(temp, g) AS m
  FROM tbl_tgeometry t1, tbl_geometry t2 WHERE t1.k % 2 = 0)
SELECT COUNT(*) FROM r WHERE getTime(temp) != CASE WHEN a IS NULL THEN getTime(m)
  WHEN m IS NULL THEN getTime(a) ELSE getTime(a) + getTime(m) END;
-- A restriction to a box keeps whole values, so it and its complement
-- partition the time and their merge is the value
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_stbox t2 WHERE temp != merge(atStbox(temp, setSRID(b, 3812)), minusStbox(temp, setSRID(b, 3812)));

-------------------------------------------------------------------------------
