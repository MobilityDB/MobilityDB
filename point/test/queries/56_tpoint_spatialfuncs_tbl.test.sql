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

SELECT round(MAX(ST_X(startValue(transform_gk(temp))))::numeric, 6) FROM tbl_tgeompoint;

SELECT round(MAX(ST_X(transform_gk(g)))::numeric, 6) FROM tbl_geompoint LIMIT 10;
SELECT round(MAX(ST_X(ST_StartPoint(transform_gk(g))))::numeric, 6) FROM tbl_geomlinestring LIMIT 10;

-------------------------------------------------------------------------------

SELECT tgeogpoint(temp) FROM tbl_tgeompoint LIMIT 10;
SELECT tgeompoint(temp) FROM tbl_tgeogpoint LIMIT 10;
SELECT tgeogpoint(temp) FROM tbl_tgeompoint3D LIMIT 10;
SELECT tgeompoint(temp) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT temp::tgeogpoint FROM tbl_tgeompoint LIMIT 10;
SELECT temp::tgeompoint FROM tbl_tgeogpoint LIMIT 10;
SELECT temp::tgeogpoint FROM tbl_tgeompoint3D LIMIT 10;
SELECT temp::tgeompoint FROM tbl_tgeogpoint3D LIMIT 10;

SELECT asText(setprecision(temp, 2)) FROM tbl_tgeompoint LIMIT 10;
SELECT asText(setprecision(temp, 2)) FROM tbl_tgeogpoint LIMIT 10;
SELECT asText(setprecision(temp, 2)) FROM tbl_tgeompoint3D LIMIT 10;
SELECT asText(setprecision(temp, 2)) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT trajectory(temp) FROM tbl_tgeompoint ORDER BY k LIMIT 10 ;
SELECT trajectory(temp) FROM tbl_tgeogpoint ORDER BY k LIMIT 10 ;
SELECT trajectory(temp) FROM tbl_tgeompoint3D ORDER BY k LIMIT 10 ;
SELECT trajectory(temp) FROM tbl_tgeogpoint3D ORDER BY k LIMIT 10 ;

SELECT round(MAX(length(temp))::numeric, 6) FROM tbl_tgeompoint;
SELECT round(MAX(length(temp))::numeric, 6) FROM tbl_tgeompoint3D;
-- Tests independent of PROJ version
SELECT count(*) FROM tbl_tgeogpoint WHERE length(temp) = ST_Length(trajectory(temp));
SELECT count(*) FROM tbl_tgeogpoint3D WHERE length(temp) = ST_Length(trajectory(temp));

SELECT round(MAX(maxValue(cumulativeLength(temp)))::numeric, 6) FROM tbl_tgeompoint;
SELECT round(MAX(maxValue(cumulativeLength(temp)))::numeric, 6) FROM tbl_tgeogpoint;
SELECT round(MAX(maxValue(cumulativeLength(temp)))::numeric, 6) FROM tbl_tgeompoint3D;
SELECT round(MAX(maxValue(cumulativeLength(temp)))::numeric, 6) FROM tbl_tgeogpoint3D;

SELECT round(MAX(maxValue(speed(temp)))::numeric, 6) FROM tbl_tgeompoint;
SELECT round(MAX(maxValue(speed(temp)))::numeric, 6) FROM tbl_tgeompoint3D;
-- Tests intended to avoid floating point precision errors
SELECT count(*) FROM tbl_tgeogpoint WHERE startValue(speed(temp)) <> 0 AND startTimestamp(temp) = startTimestamp(speed(temp)) 
AND abs(startValue(speed(temp)) - st_distance(startValue(temp), getValue(instantN(temp,2))) / EXTRACT(epoch FROM timestampN(temp,2) - startTimestamp(temp))) < 1e-5;
SELECT count(*) FROM tbl_tgeogpoint3D WHERE startValue(speed(temp)) <> 0 AND startTimestamp(temp) = startTimestamp(speed(temp)) 
AND abs(startValue(speed(temp)) - st_distance(startValue(temp), getValue(instantN(temp,2))) / EXTRACT(epoch FROM timestampN(temp,2) - startTimestamp(temp))) < 1e-5;

SELECT st_astext(twcentroid(temp)) FROM tbl_tgeompoint LIMIT 10;
SELECT st_astext(twcentroid(temp)) FROM tbl_tgeompoint3D LIMIT 10;

SELECT round(azimuth(temp), 12) FROM tbl_tgeompoint WHERE azimuth(temp) IS NOT NULL LIMIT 10;
SELECT round(azimuth(temp), 12) FROM tbl_tgeogpoint WHERE azimuth(temp) IS NOT NULL LIMIT 10;
SELECT round(azimuth(temp), 12) FROM tbl_tgeompoint3D WHERE azimuth(temp) IS NOT NULL LIMIT 10;
SELECT round(azimuth(temp), 12) FROM tbl_tgeogpoint3D WHERE azimuth(temp) IS NOT NULL LIMIT 10;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_geometry t2 WHERE
-- Modulo used to reduce time needed for the tests
t1.k % 2 = 0 AND temp != merge(atGeometry(temp, g), minusGeometry(temp, g));

SELECT COUNT(*)  FROM tbl_tgeompoint t1, tbl_stbox t2 WHERE temp != merge(atStbox(temp, b), minusStbox(temp, b));
SELECT COUNT(*)  FROM tbl_tgeogpoint t1, tbl_geodstbox t2 WHERE temp != merge(atStbox(temp, b), minusStbox(temp, b));

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

SELECT count(*) FROM tbl_tgeompoint, 
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, 
( SELECT * FROM tbl_tgeompoint t2 LIMIT 10 ) t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
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
-- SELECT count(*) FROM tbl_tgeogpoint3D, 
-- ( SELECT * FROM tbl_geography3D LIMIT 10 ) t
-- WHERE shortestLine(g, temp) IS NOT NULL;
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

--------------------------------------------------------

SELECT st_astext(asGeometry(temp, true)) FROM tbl_tgeompoint LIMIT 10;
SELECT st_astext(asGeometry(temp, true)) FROM tbl_tgeompoint3D LIMIT 10;

SELECT asGeometry(temp, true) FROM tbl_tgeompoint LIMIT 10;
SELECT asGeometry(temp, true) FROM tbl_tgeompoint3D LIMIT 10;

SELECT st_astext(asGeography(temp, true)) FROM tbl_tgeogpoint LIMIT 10;
SELECT st_astext(asGeography(temp, true)) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT asGeography(temp, true) FROM tbl_tgeogpoint LIMIT 10;
SELECT asGeography(temp, true) FROM tbl_tgeogpoint3D LIMIT 10;

-------------------------------------------------------------------------------

SELECT asText((temp::geometry)::tgeompoint) FROM tbl_tgeompoint LIMIT 10;
SELECT asText((temp::geometry)::tgeompoint) FROM tbl_tgeompoint3D LIMIT 10;

SELECT count(*) FROM tbl_tgeompoint WHERE (temp::geometry)::tgeompoint = temp;
SELECT count(*) FROM tbl_tgeompoint3D WHERE (temp::geometry)::tgeompoint = temp;

SELECT asText((temp::geography)::tgeogpoint) FROM tbl_tgeogpoint LIMIT 10;
SELECT asText((temp::geography)::tgeogpoint) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT (temp::geography)::tgeogpoint FROM tbl_tgeogpoint LIMIT 10;
SELECT (temp::geography)::tgeogpoint FROM tbl_tgeogpoint3D LIMIT 10;

-------------------------------------------------------------------------------

SELECT st_astext(geoMeasure(t1.temp, t2.temp)) FROM tbl_tgeompoint t1, tbl_tfloat t2 WHERE getTime(t1.temp) && getTime(t2.temp);
SELECT st_astext(geoMeasure(t1.temp, t2.temp)) FROM tbl_tgeompoint3D t1, tbl_tfloat t2 WHERE getTime(t1.temp) && getTime(t2.temp);

SELECT st_astext(geoMeasure(temp, round(speed(temp),2))) FROM tbl_tgeompoint WHERE speed(temp) IS NOT NULL;
SELECT st_astext(geoMeasure(temp, round(speed(temp),2))) FROM tbl_tgeompoint3D WHERE speed(temp) IS NOT NULL;

-------------------------------------------------------------------------------

-- set parallel_tuple_cost=100;
-- set parallel_setup_cost=100;
set force_parallel_mode=off;

-------------------------------------------------------------------------------
