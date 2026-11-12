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

SELECT COUNT(*) FROM tbl_tgeompoint WHERE startValue(transform(setSRID(temp, 5676), 4326)) = st_transform(st_setSRID(startValue(temp), 5676), 4326);
SELECT COUNT(*) FROM tbl_tgeompoint3D WHERE startValue(transform(setSRID(temp, 5676), 4326)) = st_transform(st_setSRID(startValue(temp), 5676), 4326);

-------------------------------------------------------------------------------
-- Transform by using Gauss Kruger Projection that is used in Secondo

SELECT ST_AsText(transformGK(geometry 'Linestring empty'));

SELECT round(MAX(ST_X(startValue(transformGK(temp)))), 6) FROM tbl_tgeompoint;

SELECT round(MAX(ST_X(transformGK(g))), 6) FROM tbl_geom_point;
SELECT round(MAX(ST_X(ST_StartPoint(transformGK(g)))), 6) FROM tbl_geom_linestring WHERE NOT ST_IsEmpty(g);

-------------------------------------------------------------------------------

SELECT tgeogpoint(temp) FROM tbl_tgeompoint LIMIT 10;
SELECT tgeompoint(temp) FROM tbl_tgeogpoint LIMIT 10;
SELECT tgeogpoint(temp) FROM tbl_tgeompoint3D LIMIT 10;
SELECT tgeompoint(temp) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT temp::tgeogpoint FROM tbl_tgeompoint LIMIT 10;
SELECT temp::tgeompoint FROM tbl_tgeogpoint LIMIT 10;
SELECT temp::tgeogpoint FROM tbl_tgeompoint3D LIMIT 10;
SELECT temp::tgeompoint FROM tbl_tgeogpoint3D LIMIT 10;

SELECT asText(round(temp, 2)) FROM tbl_tgeompoint LIMIT 10;
SELECT asText(round(temp, 2)) FROM tbl_tgeogpoint LIMIT 10;
SELECT asText(round(temp, 2)) FROM tbl_tgeompoint3D LIMIT 10;
SELECT asText(round(temp, 2)) FROM tbl_tgeogpoint3D LIMIT 10;

-- Round an array of temporal points
SELECT asText(round(array_agg(inst ORDER BY k), 2)) FROM tbl_tgeompoint_inst WHERE inst IS NOT NULL AND k % 20 = 1;
SELECT asText(round(array_agg(inst ORDER BY k), 2)) FROM tbl_tgeogpoint_inst WHERE inst IS NOT NULL AND k % 20 = 1;

SELECT round(MAX(twavg(getX(temp))), 6) FROM tbl_tgeompoint;
SELECT round(MAX(twavg(getX(temp))), 6) FROM tbl_tgeogpoint;
SELECT round(MAX(twavg(getY(temp))), 6) FROM tbl_tgeompoint;
SELECT round(MAX(twavg(getY(temp))), 6) FROM tbl_tgeogpoint;

SELECT round(MAX(twavg(getX(temp))), 6) FROM tbl_tgeompoint3D;
SELECT round(MAX(twavg(getX(temp))), 6) FROM tbl_tgeogpoint3D;
SELECT round(MAX(twavg(getY(temp))), 6) FROM tbl_tgeompoint3D;
SELECT round(MAX(twavg(getY(temp))), 6) FROM tbl_tgeogpoint3D;
SELECT round(MAX(twavg(getZ(temp))), 6) FROM tbl_tgeompoint3D;
SELECT round(MAX(twavg(getZ(temp))), 6) FROM tbl_tgeogpoint3D;

SELECT MAX(ST_NPoints(trajectory(temp))) FROM tbl_tgeompoint;
SELECT MAX(ST_NPoints(trajectory(temp)::geometry)) FROM tbl_tgeogpoint;
SELECT MAX(ST_NPoints(trajectory(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(ST_NPoints(trajectory(temp)::geometry)) FROM tbl_tgeogpoint3D;
-- Apply ST_UnaryUnion to the result
SELECT MAX(ST_NPoints(trajectory(temp, true))) FROM tbl_tgeompoint;
SELECT MAX(ST_NPoints(trajectory(temp, true))) FROM tbl_tgeompoint3D;

SELECT round(MAX(length(temp)), 6) FROM tbl_tgeompoint;
SELECT round(MAX(length(temp)), 6) FROM tbl_tgeompoint3D;
-- Tests independent of PROJ version
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE length(temp) = ST_Length(trajectory(temp));
SELECT COUNT(*) FROM tbl_tgeogpoint3D WHERE length(temp) = ST_Length(trajectory(temp));

SELECT round(MAX(maxValue(cumulativeLength(temp))), 6) FROM tbl_tgeompoint;
SELECT round(MAX(maxValue(cumulativeLength(temp))), 6) FROM tbl_tgeogpoint;
SELECT round(MAX(maxValue(cumulativeLength(temp))), 6) FROM tbl_tgeompoint3D;
SELECT round(MAX(maxValue(cumulativeLength(temp))), 6) FROM tbl_tgeogpoint3D;

SELECT MAX(ST_Area(convexHull(temp))) FROM tbl_tgeompoint;

SELECT round(MAX(maxValue(speed(temp))), 6) FROM tbl_tgeompoint WHERE interp(temp) = 'Linear';
SELECT round(MAX(maxValue(speed(temp))), 6) FROM tbl_tgeompoint3D WHERE interp(temp) = 'Linear';
-- Tests intended to avoid floating point precision errors
SELECT COUNT(*) FROM tbl_tgeogpoint
WHERE interp(temp) = 'Linear' AND startValue(speed(temp)) <> 0 AND startTimestamp(temp) = startTimestamp(speed(temp))
AND abs(startValue(speed(temp)) - st_distance(startValue(temp), getValue(instantN(temp,2))) / EXTRACT(epoch FROM timestampN(temp,2) - startTimestamp(temp))) < 1e-5;
SELECT COUNT(*) FROM tbl_tgeogpoint3D
WHERE interp(temp) = 'Linear' AND startValue(speed(temp)) <> 0 AND startTimestamp(temp) = startTimestamp(speed(temp))
AND abs(startValue(speed(temp)) - st_distance(startValue(temp), getValue(instantN(temp,2))) / EXTRACT(epoch FROM timestampN(temp,2) - startTimestamp(temp))) < 1e-5;

SELECT MAX(length(ST_AsText(round(twcentroid(temp), 6)))) FROM tbl_tgeompoint;
SELECT MAX(length(ST_AsText(round(twcentroid(temp), 6)))) FROM tbl_tgeompoint3D;

SELECT round(AVG(degrees(direction(temp))), 6) FROM tbl_tgeompoint;
SELECT round(AVG(degrees(direction(temp))), 6) FROM tbl_tgeogpoint;
SELECT round(AVG(degrees(direction(temp))), 6) FROM tbl_tgeompoint3D;
SELECT round(AVG(degrees(direction(temp))), 6) FROM tbl_tgeogpoint3D;

SELECT COUNT(*) FROM tbl_tgeompoint WHERE azimuth(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D WHERE azimuth(temp) IS NOT NULL;
-- Return negative result in PostGIS 2.5.5, return erroneous value in PostGIS 3.1.1
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE azimuth(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D WHERE azimuth(temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_float t1, tbl_float t2 WHERE angularDifference(t1.f, t2.f) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint WHERE angularDifference(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D WHERE angularDifference(temp) IS NOT NULL;

-------------------------------------------------------------------------------

-- 2D
SELECT MAX(maxValue(round(degrees(bearing(g, temp)), 6))) FROM tbl_geom_point t1, tbl_tgeompoint t2
WHERE bearing(g, temp) IS NOT NULL;
SELECT MAX(maxValue(round(degrees(bearing(temp, g)), 6))) FROM tbl_tgeompoint t1, tbl_geom_point t2
WHERE bearing(temp, g) IS NOT NULL;
SELECT MAX(maxValue(round(degrees(bearing(t1.temp, t2.temp)), 6))) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE bearing(t1.temp, t2.temp) IS NOT NULL;

SELECT MAX(maxValue(round(degrees(bearing(g, temp)), 6))) FROM tbl_geog_point t1, tbl_tgeogpoint t2
WHERE bearing(g, temp) IS NOT NULL;
SELECT MAX(maxValue(round(degrees(bearing(temp, g)), 6))) FROM tbl_tgeogpoint t1, tbl_geog_point t2
WHERE bearing(temp, g) IS NOT NULL;
-- SELECT MAX(maxValue(round(degrees(bearing(t1.temp, t2.temp)), 6))) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
-- WHERE bearing(t1.temp, t2.temp) IS NOT NULL;

-- 3D
SELECT MAX(maxValue(round(degrees(bearing(g, temp)), 6))) FROM tbl_geom_point3D t1, tbl_tgeompoint3D t2
WHERE bearing(g, temp) IS NOT NULL;
SELECT MAX(maxValue(round(degrees(bearing(temp, g)), 6))) FROM tbl_tgeompoint3D t1, tbl_geom_point3D t2
WHERE bearing(temp, g) IS NOT NULL;
SELECT MAX(maxValue(round(degrees(bearing(t1.temp, t2.temp)), 6))) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE bearing(t1.temp, t2.temp) IS NOT NULL;

SELECT MAX(maxValue(round(degrees(bearing(g, temp)), 6))) FROM tbl_geog_point3D t1, tbl_tgeogpoint3D t2
WHERE bearing(g, temp) IS NOT NULL;
SELECT MAX(maxValue(round(degrees(bearing(temp, g)), 6))) FROM tbl_tgeogpoint3D t1, tbl_geog_point3D t2
WHERE bearing(temp, g) IS NOT NULL;
-- SELECT MAX(maxValue(round(degrees(bearing(t1.temp, t2.temp)), 6))) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
-- WHERE bearing(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeompoint WHERE isSimple(temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D WHERE isSimple(temp);

SELECT SUM(numInstants(t)) FROM tbl_tgeompoint, unnest(makeSimple(temp)) t;
SELECT SUM(numInstants(t)) FROM tbl_tgeompoint3D, unnest(makeSimple(temp)) t;
SELECT DISTINCT merge(makeSimple(temp)) = temp from tbl_tgeompoint;
SELECT DISTINCT merge(makeSimple(temp)) = temp from tbl_tgeompoint3D;

-------------------------------------------------------------------------------
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_geom t2 WHERE temp != merge(atGeometry(temp, g), minusGeometry(temp, g));
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_geography t2 WHERE g IS NOT NULL AND NOT ST_IsEmpty(g::geometry) AND geometrytype(g::geometry) IN ('POLYGON', 'MULTIPOLYGON') AND temp != merge(atGeometry(temp, g), minusGeometry(temp, g));

SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_stbox t2 WHERE temp != merge(atStbox(temp, b), minusStbox(temp, b));
SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_stbox3d t2 WHERE temp != merge(atStbox(temp, b), minusStbox(temp, b));
-- TODO THE FOLLOWING FUNCTION RETURNS 1, WE NEED TO DEBUG TO UNDERSTAND WHY
SELECT COUNT(*) FROM tbl_tgeogpoint3d t1, tbl_geodstbox3d t2 WHERE temp != merge(atStbox(temp, b), minusStbox(temp, b));

SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_floatspan t2 WHERE temp != merge(atElevation(temp, f), minusElevation(temp, f));
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_floatspan t2 WHERE temp != merge(atElevation(temp, f), minusElevation(temp, f));


-------------------------------------------------------------------------------
