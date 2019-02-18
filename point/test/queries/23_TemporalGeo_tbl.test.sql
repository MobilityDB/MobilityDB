-------------------------------------------------------------------------------

SELECT astext(temp) FROM tbl_tgeompoint;
SELECT astext(temp) FROM tbl_tgeogpoint;
SELECT astext(temp) FROM tbl_tgeompoint3D;
SELECT astext(temp) FROM tbl_tgeogpoint3D;

SELECT asEWKT(temp) FROM tbl_tgeompoint;
SELECT asEWKT(temp) FROM tbl_tgeogpoint;
SELECT asEWKT(temp) FROM tbl_tgeompoint3D;
SELECT asEWKT(temp) FROM tbl_tgeogpoint3D;

SELECT k%10, astext(array_agg(g)) FROM tbl_geomcollection GROUP BY k%10;
SELECT k%10, astext(array_agg(g)) FROM tbl_geogcollection GROUP BY k%10;
SELECT k%10, astext(array_agg(g)) FROM tbl_geomcollection3D GROUP BY k%10;
SELECT k%10, astext(array_agg(g)) FROM tbl_geogcollection3D GROUP BY k%10;

SELECT k%10, asEWKT(array_agg(g)) FROM tbl_geomcollection GROUP BY k%10;
SELECT k%10, asEWKT(array_agg(g)) FROM tbl_geogcollection GROUP BY k%10;
SELECT k%10, asEWKT(array_agg(g)) FROM tbl_geomcollection3D GROUP BY k%10;
SELECT k%10, asEWKT(array_agg(g)) FROM tbl_geogcollection3D GROUP BY k%10;

SELECT k%10, astext(array_agg(temp)) FROM tbl_tgeompoint GROUP BY k%10;
SELECT k%10, astext(array_agg(temp)) FROM tbl_tgeogpoint GROUP BY k%10;
SELECT k%10, astext(array_agg(temp)) FROM tbl_tgeompoint3D GROUP BY k%10;
SELECT k%10, astext(array_agg(temp)) FROM tbl_tgeogpoint3D GROUP BY k%10;

SELECT k%10, asEWKT(array_agg(temp)) FROM tbl_tgeompoint GROUP BY k%10;
SELECT k%10, asEWKT(array_agg(temp)) FROM tbl_tgeogpoint GROUP BY k%10;
SELECT k%10, asEWKT(array_agg(temp)) FROM tbl_tgeompoint3D GROUP BY k%10;
SELECT k%10, asEWKT(array_agg(temp)) FROM tbl_tgeogpoint3D GROUP BY k%10;

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
SELECT tgeogpoint(temp) FROM tbl_tgeompoint;
SELECT tgeompoint(temp) FROM tbl_tgeogpoint;
SELECT tgeogpoint(temp) FROM tbl_tgeompoint3D;
SELECT tgeompoint(temp) FROM tbl_tgeogpoint3D;

SELECT temp::tgeogpoint FROM tbl_tgeompoint;
SELECT temp::tgeompoint FROM tbl_tgeogpoint;
SELECT temp::tgeogpoint FROM tbl_tgeompoint3D;
SELECT temp::tgeompoint FROM tbl_tgeogpoint3D;

SELECT synctrajectory(seq) FROM tbl_tgeompointseq;
SELECT synctrajectory(ts) FROM tbl_tgeompoints;
SELECT synctrajectory(seq) FROM tbl_tgeompoint3Dseq;
SELECT synctrajectory(ts) FROM tbl_tgeompoint3Ds;

SELECT synctrajectorypers(seq) FROM tbl_tgeompointseq;
SELECT synctrajectorypers(ts) FROM tbl_tgeompoints;
SELECT synctrajectorypers(seq) FROM tbl_tgeompoint3Dseq;
SELECT synctrajectorypers(ts) FROM tbl_tgeompoint3Ds;

SELECT trajectory(temp) FROM tbl_tgeompoint;
SELECT trajectory(temp) FROM tbl_tgeogpoint;
SELECT trajectory(temp) FROM tbl_tgeompoint3D;
SELECT trajectory(temp) FROM tbl_tgeogpoint3D;

SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection
WHERE atGeometry(temp, g) IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection
WHERE minusGeometry(temp, g) IS NOT NULL;

SELECT azimuth(seq) FROM tbl_tgeompointseq;
SELECT azimuth(ts) FROM tbl_tgeompoints;
SELECT azimuth(seq) FROM tbl_tgeogpointseq;
SELECT azimuth(ts) FROM tbl_tgeogpoints;

SELECT azimuth(seq) FROM tbl_tgeompoint3Dseq;
SELECT azimuth(ts) FROM tbl_tgeompoint3Ds;
SELECT azimuth(seq) FROM tbl_tgeogpoint3Dseq;
SELECT azimuth(ts) FROM tbl_tgeogpoint3Ds;

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

SELECT st_astext(temp::geometry) FROM tbl_tgeompoint;
SELECT st_astext(temp::geometry) FROM tbl_tgeompoint3D;

SELECT temp::geometry FROM tbl_tgeompoint;
SELECT temp::geometry FROM tbl_tgeompoint3D;

SELECT st_astext(temp::geography) FROM tbl_tgeogpoint;
SELECT st_astext(temp::geography) FROM tbl_tgeogpoint3D;

SELECT temp::geography FROM tbl_tgeogpoint;
SELECT temp::geography FROM tbl_tgeogpoint3D;

-------------------------------------------------------------------------------

SELECT astext((temp::geometry)::tgeompoint) FROM tbl_tgeompoint;
SELECT astext((temp::geometry)::tgeompoint) FROM tbl_tgeompoint3D;

SELECT count(*) FROM tbl_tgeompoint WHERE (temp::geometry)::tgeompoint = temp;
SELECT count(*) FROM tbl_tgeompoint3D WHERE (temp::geometry)::tgeompoint = temp;

SELECT astext((temp::geography)::tgeogpoint) FROM tbl_tgeogpoint;
SELECT astext((temp::geography)::tgeogpoint) FROM tbl_tgeogpoint3D;

SELECT (temp::geography)::tgeogpoint FROM tbl_tgeogpoint;
SELECT (temp::geography)::tgeogpoint FROM tbl_tgeogpoint3D;

-------------------------------------------------------------------------------

