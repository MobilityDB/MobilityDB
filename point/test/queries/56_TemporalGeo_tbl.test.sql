-------------------------------------------------------------------------------

SELECT astext(temp) FROM tbl_tgeompoint LIMIT 10;
SELECT astext(temp) FROM tbl_tgeogpoint LIMIT 10;
SELECT astext(temp) FROM tbl_tgeompoint3D LIMIT 10;
SELECT astext(temp) FROM tbl_tgeogpoint3D LIMIT 10;
SELECT k%90, astext(array_agg(g ORDER BY k)) FROM tbl_geogcollection3D GROUP BY k%90 ORDER BY k%90 LIMIT 10;
SELECT k%90, astext(array_agg(temp ORDER BY k)) FROM tbl_tgeogpoint3D GROUP BY k%90 ORDER BY k%90 LIMIT 10;

SELECT asEWKT(temp) FROM tbl_tgeompoint LIMIT 10;
SELECT asEWKT(temp) FROM tbl_tgeogpoint LIMIT 10;
SELECT asEWKT(temp) FROM tbl_tgeompoint3D LIMIT 10;
SELECT asEWKT(temp) FROM tbl_tgeogpoint3D LIMIT 10;
SELECT k%90, asEWKT(array_agg(g ORDER BY k)) FROM tbl_geogcollection3D GROUP BY k%90 ORDER BY k%90 LIMIT 10;
SELECT k%90, asEWKT(array_agg(temp ORDER BY k)) FROM tbl_tgeogpoint3D GROUP BY k%90 ORDER BY k%90 LIMIT 10;

SELECT DISTINCT SRID(temp) FROM tbl_tgeompoint;
SELECT DISTINCT SRID(temp) FROM tbl_tgeogpoint;
SELECT DISTINCT SRID(temp) FROM tbl_tgeompoint3D;
SELECT DISTINCT SRID(temp) FROM tbl_tgeogpoint3D;

/*
SELECT setSRID(temp,5676) FROM tbl_tgeompoint;
SELECT setSRID(temp,4326) FROM tbl_tgeogpoint;
SELECT setSRID(temp,5676) FROM tbl_tgeompoint3D;
SELECT setSRID(temp,4326) FROM tbl_tgeogpoint3D;
*/

SELECT count(*) FROM tbl_tgeompoint WHERE startValue(transform(setSRID(temp, 5676), 4326)) = st_transform(st_setSRID(startValue(temp), 5676), 4326);
SELECT count(*) FROM tbl_tgeompoint3D WHERE startValue(transform(setSRID(temp, 5676), 4326)) = st_transform(st_setSRID(startValue(temp), 5676), 4326);

-------------------------------------------------------------------------------
-- Transform by using Gauss Kruger Projection that is used in Secondo

SELECT transform_gk(temp) from tbl_tgeompoint LIMIT 10;

SELECT transform_gk(g) from tbl_geompoint where not st_isempty(g) LIMIT 10;
SELECT transform_gk(g) from tbl_geomlinestring where not st_isempty(g) LIMIT 10;

-------------------------------------------------------------------------------

SELECT tgeogpoint(temp) FROM tbl_tgeompoint LIMIT 10;
SELECT tgeompoint(temp) FROM tbl_tgeogpoint LIMIT 10;
SELECT tgeogpoint(temp) FROM tbl_tgeompoint3D LIMIT 10;
SELECT tgeompoint(temp) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT temp::tgeogpoint FROM tbl_tgeompoint LIMIT 10;
SELECT temp::tgeompoint FROM tbl_tgeogpoint LIMIT 10;
SELECT temp::tgeogpoint FROM tbl_tgeompoint3D LIMIT 10;
SELECT temp::tgeompoint FROM tbl_tgeogpoint3D LIMIT 10;

SELECT trajectory(temp) FROM tbl_tgeompoint ORDER BY k LIMIT 10 ;
SELECT trajectory(temp) FROM tbl_tgeogpoint ORDER BY k LIMIT 10 ;
SELECT trajectory(temp) FROM tbl_tgeompoint3D ORDER BY k LIMIT 10 ;
SELECT trajectory(temp) FROM tbl_tgeogpoint3D ORDER BY k LIMIT 10 ;

SELECT MAX(length(temp)) FROM tbl_tgeompoint;
SELECT MAX(length(temp)) FROM tbl_tgeogpoint;
SELECT MAX(length(temp)) FROM tbl_tgeompoint3D;
SELECT MAX(length(temp)) FROM tbl_tgeompoint3D;

SELECT MAX(maxValue(cumulativeLength(temp))) FROM tbl_tgeompoint;
SELECT MAX(maxValue(cumulativeLength(temp))) FROM tbl_tgeogpoint;
SELECT MAX(maxValue(cumulativeLength(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(maxValue(cumulativeLength(temp))) FROM tbl_tgeompoint3D;

SELECT MAX(maxValue(speed(temp))) FROM tbl_tgeompoint;
SELECT MAX(maxValue(speed(temp))) FROM tbl_tgeogpoint;
SELECT MAX(maxValue(speed(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(maxValue(speed(temp))) FROM tbl_tgeompoint3D;

SELECT st_astext(twcentroid(temp)) FROM tbl_tgeompoint LIMIT 10;
SELECT st_astext(twcentroid(temp)) FROM tbl_tgeompoint3D LIMIT 10;

SELECT round(azimuth(temp), 13) FROM tbl_tgeompoint WHERE azimuth(temp) IS NOT NULL LIMIT 10;
SELECT round(azimuth(temp), 13) FROM tbl_tgeogpoint azimuth(temp) IS NOT NULL LIMIT 10;
SELECT round(azimuth(temp), 13) FROM tbl_tgeompoint3D azimuth(temp) IS NOT NULL LIMIT 10;
SELECT round(azimuth(temp), 13) FROM tbl_tgeompoint3D azimuth(temp) IS NOT NULL LIMIT 10;

-------------------------------------------------------------------------------

SELECT atGeometry(temp, g) FROM tbl_tgeompoint, tbl_geomcollection
WHERE atGeometry(temp, g) IS NOT NULL AND atGeometry(temp, g) != temp LIMIT 10;

SELECT minusGeometry(temp, g) FROM tbl_tgeompoint, tbl_geomcollection
WHERE minusGeometry(temp, g) IS NOT NULL AND minusGeometry(temp, g) != temp LIMIT 10;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection 
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogcollection
WHERE NearestApproachInstant(temp, g) IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection 
WHERE nearestApproachDistance(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 
WHERE nearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D, tbl_geomcollection3D
WHERE nearestApproachDistance(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 
WHERE nearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection 
WHERE g |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 
WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D, tbl_geomcollection3D 
WHERE g |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tgeogpoint, tbl_geogcollection 
WHERE nearestApproachDistance(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 
WHERE nearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geogcollection3D
WHERE nearestApproachDistance(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 
WHERE nearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_tgeogpoint, tbl_geogcollection 
WHERE g |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 
WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geogcollection3D 
WHERE g |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D, tbl_geomcollection3D
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_tgeogpoint, tbl_geogcollection
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geogcollection3D
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

--------------------------------------------------------

SELECT st_astext(temp::geometry) FROM tbl_tgeompoint LIMIT 10;
SELECT st_astext(temp::geometry) FROM tbl_tgeompoint3D LIMIT 10;

SELECT temp::geometry FROM tbl_tgeompoint LIMIT 10;
SELECT temp::geometry FROM tbl_tgeompoint3D LIMIT 10;

SELECT st_astext(temp::geography) FROM tbl_tgeogpoint LIMIT 10;
SELECT st_astext(temp::geography) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT temp::geography FROM tbl_tgeogpoint LIMIT 10;
SELECT temp::geography FROM tbl_tgeogpoint3D LIMIT 10;

-------------------------------------------------------------------------------

SELECT astext((temp::geometry)::tgeompoint) FROM tbl_tgeompoint LIMIT 10;
SELECT astext((temp::geometry)::tgeompoint) FROM tbl_tgeompoint3D LIMIT 10;

SELECT count(*) FROM tbl_tgeompoint WHERE (temp::geometry)::tgeompoint = temp;
SELECT count(*) FROM tbl_tgeompoint3D WHERE (temp::geometry)::tgeompoint = temp;

SELECT astext((temp::geography)::tgeogpoint) FROM tbl_tgeogpoint LIMIT 10;
SELECT astext((temp::geography)::tgeogpoint) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT (temp::geography)::tgeogpoint FROM tbl_tgeogpoint LIMIT 10;
SELECT (temp::geography)::tgeogpoint FROM tbl_tgeogpoint3D LIMIT 10;

-------------------------------------------------------------------------------

