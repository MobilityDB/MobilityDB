-------------------------------------------------------------------------------

-- set parallel_tuple_cost=0;
-- set parallel_setup_cost=0;
set force_parallel_mode=regress;

-------------------------------------------------------------------------------

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

SELECT MAX(ST_X(startValue(transform_gk(temp)))) from tbl_tgeompoint;

SELECT MAX(ST_X(transform_gk(g))) from tbl_geompoint LIMIT 10;
SELECT MAX(ST_X(ST_StartPoint(transform_gk(g)))) from tbl_geomlinestring LIMIT 10;

-------------------------------------------------------------------------------

SELECT tgeogpoint(temp) FROM tbl_tgeompoint LIMIT 10;
SELECT tgeompoint(temp) FROM tbl_tgeogpoint LIMIT 10;
SELECT tgeogpoint(temp) FROM tbl_tgeompoint3D LIMIT 10;
SELECT tgeompoint(temp) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT temp::tgeogpoint FROM tbl_tgeompoint LIMIT 10;
SELECT temp::tgeompoint FROM tbl_tgeogpoint LIMIT 10;
SELECT temp::tgeogpoint FROM tbl_tgeompoint3D LIMIT 10;
SELECT temp::tgeompoint FROM tbl_tgeogpoint3D LIMIT 10;

SELECT astext(setprecision(temp, 2)) FROM tbl_tgeompoint LIMIT 10;
SELECT astext(setprecision(temp, 2)) FROM tbl_tgeogpoint LIMIT 10;
SELECT astext(setprecision(temp, 2)) FROM tbl_tgeompoint3D LIMIT 10;
SELECT astext(setprecision(temp, 2)) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT trajectory(temp) FROM tbl_tgeompoint ORDER BY k LIMIT 10 ;
SELECT trajectory(temp) FROM tbl_tgeogpoint ORDER BY k LIMIT 10 ;
SELECT trajectory(temp) FROM tbl_tgeompoint3D ORDER BY k LIMIT 10 ;
SELECT trajectory(temp) FROM tbl_tgeogpoint3D ORDER BY k LIMIT 10 ;

SELECT MAX(length(temp)) FROM tbl_tgeompoint;
SELECT MAX(length(temp)) FROM tbl_tgeompoint3D;
-- Tests independent of PROJ version
SELECT count(*) FROM tbl_tgeogpoint WHERE length(temp) = ST_Length(trajectory(temp));
SELECT count(*) FROM tbl_tgeogpoint3D WHERE length(temp) = ST_Length(trajectory(temp));

SELECT MAX(maxValue(cumulativeLength(temp))) FROM tbl_tgeompoint;
SELECT MAX(maxValue(cumulativeLength(temp))) FROM tbl_tgeogpoint;
SELECT MAX(maxValue(cumulativeLength(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(maxValue(cumulativeLength(temp))) FROM tbl_tgeogpoint3D;

SELECT MAX(maxValue(speed(temp))) FROM tbl_tgeompoint;
SELECT MAX(maxValue(speed(temp))) FROM tbl_tgeompoint3D;
-- Tests intended to avoid floating point precision errors
SELECT count(*) FROM tbl_tgeogpoint where startValue(speed(temp)) <> 0 AND startTimestamp(temp) = startTimestamp(speed(temp)) 
AND abs(startValue(speed(temp)) - st_distance(startValue(temp), getValue(instantN(temp,2))) / EXTRACT(epoch FROM timestampN(temp,2) - startTimestamp(temp))) < 1e-5;
SELECT count(*) FROM tbl_tgeogpoint3D where startValue(speed(temp)) <> 0 AND startTimestamp(temp) = startTimestamp(speed(temp)) 
AND abs(startValue(speed(temp)) - st_distance(startValue(temp), getValue(instantN(temp,2))) / EXTRACT(epoch FROM timestampN(temp,2) - startTimestamp(temp))) < 1e-5;

SELECT st_astext(twcentroid(temp)) FROM tbl_tgeompoint LIMIT 10;
SELECT st_astext(twcentroid(temp)) FROM tbl_tgeompoint3D LIMIT 10;

SELECT round(azimuth(temp), 12) FROM tbl_tgeompoint WHERE azimuth(temp) IS NOT NULL LIMIT 10;
SELECT round(azimuth(temp), 12) FROM tbl_tgeogpoint WHERE azimuth(temp) IS NOT NULL LIMIT 10;
SELECT round(azimuth(temp), 12) FROM tbl_tgeompoint3D WHERE azimuth(temp) IS NOT NULL LIMIT 10;
SELECT round(azimuth(temp), 12) FROM tbl_tgeogpoint3D WHERE azimuth(temp) IS NOT NULL LIMIT 10;

-------------------------------------------------------------------------------

SELECT atGeometry(temp, g) FROM tbl_tgeompoint, tbl_geometry
WHERE atGeometry(temp, g) IS NOT NULL AND atGeometry(temp, g) != temp LIMIT 10;

SELECT minusGeometry(temp, g) FROM tbl_tgeompoint, tbl_geometry
WHERE minusGeometry(temp, g) IS NOT NULL AND minusGeometry(temp, g) != temp LIMIT 10;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tgeompoint, 
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, 
( SELECT * FROM tbl_tgeompoint t2 LIMIT 10 ) t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
/* Errors */ 
SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D 
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_tgeogpoint, 
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1, 
( SELECT * FROM tbl_tgeogpoint t2  LIMIT 10 ) t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
/* Errors */ 
SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geography3D 
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint, 
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, 
( SELECT * FROM tbl_tgeompoint t2 LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D, 
( SELECT * FROM tbl_geometry3D LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D t1, 
( SELECT * FROM tbl_tgeompoint3D LIMIT 10 ) t2 
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_tgeogpoint, 
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1, 
( SELECT * FROM tbl_tgeogpoint t2 LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D, 
( SELECT * FROM tbl_geography3D LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D t1, 
( SELECT * FROM tbl_tgeogpoint3D LIMIT 10 ) t2 
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint, 
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, 
( SELECT * FROM tbl_tgeompoint t2 LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D, 
( SELECT * FROM tbl_geometry3D LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D t1, 
(SELECT * FROM tbl_tgeompoint3D LIMIT 10 ) t2 
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tgeogpoint, 
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1, 
( SELECT * FROM tbl_tgeogpoint t2 LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D, 
( SELECT * FROM tbl_geography3D LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D t1, 
(SELECT * FROM tbl_tgeogpoint3D LIMIT 10 ) t2 
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint, 
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, 
( SELECT * FROM tbl_tgeompoint t2 LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D, 
( SELECT * FROM tbl_geometry3D LIMIT 10 ) t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D t1, 
( SELECT * FROM tbl_tgeompoint3D LIMIT 10 ) t2 
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_tgeogpoint, 
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1, 
( SELECT * FROM tbl_tgeogpoint t2 LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D, 
( SELECT * FROM tbl_geography3D LIMIT 10 ) t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D t1, 
( SELECT * FROM tbl_tgeogpoint3D LIMIT 10 ) t2 
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

-- set parallel_tuple_cost=100;
-- set parallel_setup_cost=100;
set force_parallel_mode=off;

-------------------------------------------------------------------------------
