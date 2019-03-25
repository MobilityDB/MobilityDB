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

SELECT transform(setSRID(temp, 5676), 4326) FROM tbl_tgeompoint;
SELECT transform(setSRID(temp, 5676), 4326) FROM tbl_tgeompoint3D;
*/

/*****************************************************************************/
-- Transform by using Gauss Kruger Projection that is used in Secondo

-- Temporal type

SELECT transform_gk(tgeompoint 'Point(13.43593 52.41721)@2018-12-20');
SELECT transform_gk(tgeompoint('Point(13.43593 52.41721)@2018-12-20'));

SELECT astext(transform_gk(tgeompoint 'Point(13.43593 52.41721)@2018-12-20'));
SELECT astext(transform_gk(tgeompoint('Point(13.43593 52.41721)@2018-12-20')));

SELECT transform_gk(tgeompoint(Instant) 'Point(13.43593 52.41721)@2018-12-20');
SELECT transform_gk(tgeompoint(InstantSet) '{Point(13.43593 52.41721)@2018-12-20 10:00:00, Point(13.43605 52.41723)@2018-12-20 10:01:00}');
SELECT transform_gk(tgeompoint(Sequence) '[Point(13.43593 52.41721)@2018-12-20 10:00:00, Point(13.43605 52.41723)@2018-12-20 10:01:00]');
SELECT transform_gk(tgeompoint(SequenceSet) '{[Point(13.43593 52.41721)@2018-12-20 10:00:00, Point(13.43605 52.41723)@2018-12-20 10:01:00],[Point(13.43705 52.41724)@2018-12-20 10:02:00,Point(13.43805 52.41730)@2018-12-20 10:03:00]}');


-- PostGIS geometry

--POINTTYPE
SELECT transform_gk(ST_MakePoint(13.43593,52.41721));
SELECT ST_AsText(transform_gk(ST_MakePoint(13.43593,52.41721)));

--LINETYPE
SELECT transform_gk(ST_MakeLine(ST_MakePoint(13.43593,52.41721)));
SELECT transform_gk(ST_MakeLine(ST_MakePoint(13.43593,52.41721),ST_MakeLine(ST_MakePoint(13.43605,52.41723))));
SELECT transform_gk(ST_MakeLine(ARRAY[ST_MakePoint(13.43593,52.41721),ST_MakePoint(13.43593,52.41723)]));

SELECT ST_AsText(transform_gk(ST_MakeLine(ST_MakePoint(13.43593,52.41721))));
SELECT ST_AsText(transform_gk(ST_MakeLine(ST_MakePoint(13.43593,52.41721),ST_MakeLine(ST_MakePoint(13.43605,52.41723)))));
SELECT ST_AsText(transform_gk(ST_MakeLine(ARRAY[ST_MakePoint(13.43593,52.41721),ST_MakePoint(13.43593,52.41723)])));

/*****************************************************************************/
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

SELECT atGeometry(temp, g) FROM tbl_tgeompoint, tbl_geomcollection
WHERE atGeometry(temp, g) IS NOT NULL AND atGeometry(temp, g) != temp LIMIT 10;

SELECT minusGeometry(temp, g) FROM tbl_tgeompoint, tbl_geomcollection
WHERE minusGeometry(temp, g) IS NOT NULL AND minusGeometry(temp, g) != temp LIMIT 10;

SELECT temporal_round(azimuth(seq), 13) FROM tbl_tgeompointseq LIMIT 10;
SELECT temporal_round(azimuth(ts), 13) FROM tbl_tgeompoints LIMIT 10;
SELECT temporal_round(azimuth(seq), 13) FROM tbl_tgeogpointseq LIMIT 10;
SELECT temporal_round(azimuth(ts), 13) FROM tbl_tgeogpoints LIMIT 10;

SELECT temporal_round(azimuth(seq), 13) FROM tbl_tgeompoint3Dseq LIMIT 10;
SELECT temporal_round(azimuth(ts), 13) FROM tbl_tgeompoint3Ds LIMIT 10;
SELECT temporal_round(azimuth(seq), 13) FROM tbl_tgeogpoint3Dseq LIMIT 10;
SELECT temporal_round(azimuth(ts), 13) FROM tbl_tgeogpoint3Ds LIMIT 10;

SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection 
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D, tbl_geomcollection3D 
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 
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

SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D, tbl_geomcollection3D
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 
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

