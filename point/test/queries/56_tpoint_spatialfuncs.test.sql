-------------------------------------------------------------------------------

-- Function added to PostGIS to interpolate a point along a geography line
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0, false, true));
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.1, false, true));
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.1, true, true));
/* Errors */
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 2, false, true));
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Point(4.35 50.85)', 0.5, false, true));

-------------------------------------------------------------------------------
-- 2D
SELECT SRID(tgeompoint 'Point(1 1)@2000-01-01');
SELECT SRID(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT SRID(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT SRID(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT SRID(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT SRID(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT SRID(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT SRID(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
-- 3D
SELECT SRID(tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT SRID(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT SRID(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT SRID(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT SRID(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT SRID(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT SRID(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT SRID(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

SELECT asEWKT(setSRID(tgeompoint 'Point(1 1 1)@2000-01-01', 5676));
SELECT asEWKT(setSRID(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 5676));
SELECT asEWKT(setSRID(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 5676));
SELECT asEWKT(setSRID(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 5676));
SELECT asEWKT(setSRID(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', 4269));
SELECT asEWKT(setSRID(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 4269));
SELECT asEWKT(setSRID(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 4269));
SELECT asEWKT(setSRID(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 4269));

SELECT startValue(transform(setSRID(tgeompoint 'Point(1 1 1)@2000-01-01', 5676), 4326)) = st_transform(geometry 'SRID=5676;Point(1 1 1)', 4326);
SELECT startValue(transform(setSRID(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 5676), 4326)) = st_transform(geometry 'SRID=5676;Point(1 1 1)', 4326);
SELECT startValue(transform(setSRID(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 5676), 4326)) = st_transform(geometry 'SRID=5676;Point(1 1 1)', 4326);
SELECT startValue(transform(setSRID(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 5676), 4326)) = st_transform(geometry 'SRID=5676;Point(1 1 1)', 4326);

--------------------------------------------------------

-- Temporal type
SELECT asEWKT(transform_gk(tgeompoint 'Point(13.43593 52.41721)@2018-12-20'));
SELECT asEWKT(transform_gk(tgeompoint '{Point(13.43593 52.41721)@2018-12-20 10:00:00, Point(13.43605 52.41723)@2018-12-20 10:01:00}'));
SELECT asEWKT(transform_gk(tgeompoint '[Point(13.43593 52.41721)@2018-12-20 10:00:00, Point(13.43605 52.41723)@2018-12-20 10:01:00]'));
SELECT asEWKT(transform_gk(tgeompoint '{[Point(13.43593 52.41721)@2018-12-20 10:00:00, Point(13.43605 52.41723)@2018-12-20 10:01:00],[Point(13.43705 52.41724)@2018-12-20 10:02:00,Point(13.43805 52.41730)@2018-12-20 10:03:00]}'));

-- PostGIS geometry
SELECT ST_AsText(transform_gk(geometry 'Point Empty'));
SELECT ST_AsText(transform_gk(geometry 'Point(13.43593 52.41721)'));
SELECT ST_AsText(geometry 'Linestring empty');
SELECT ST_AsText(transform_gk(geometry 'Linestring(13.43593 52.41721,13.43593 52.41723)'));

/* Error */
SELECT transform_gk(geometry 'Polygon((0 0,0 10,10 10,10 0,0 0))');

--------------------------------------------------------

-- 2D
SELECT asText(setprecision(tgeompoint 'Point(1.12345 1.12345)@2000-01-01', 2));
SELECT asText(setprecision(tgeompoint '{Point(1.12345 1.12345)@2000-01-01, Point(2 2)@2000-01-02, Point(1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(setprecision(tgeompoint '[Point(1.12345 1.12345)@2000-01-01, Point(2 2)@2000-01-02, Point(1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(setprecision(tgeompoint '{[Point(1.12345 1.12345)@2000-01-01, Point(2 2)@2000-01-02, Point(1.12345 1.12345)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2));
SELECT asText(setprecision(tgeogpoint 'Point(1.12345 1.12345)@2000-01-01', 2));
SELECT asText(setprecision(tgeogpoint '{Point(1.12345 1.12345)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(setprecision(tgeogpoint '[Point(1.12345 1.12345)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(setprecision(tgeogpoint '{[Point(1.12345 1.12345)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.12345 1.12345)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2));
-- 3D
SELECT asText(setprecision(tgeompoint 'Point(1.12345 1.12345 1.12345)@2000-01-01', 2));
SELECT asText(setprecision(tgeompoint '{Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(setprecision(tgeompoint '[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(setprecision(tgeompoint '{[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2));
SELECT asText(setprecision(tgeogpoint 'Point(1.12345 1.12345 1.12345)@2000-01-01', 2));
SELECT asText(setprecision(tgeogpoint '{Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(setprecision(tgeogpoint '[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(setprecision(tgeogpoint '{[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2));

--------------------------------------------------------

-- 2D
SELECT ST_AsText(trajectory(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT ST_AsText(trajectory(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT ST_AsText(trajectory(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsText(trajectory(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsText(trajectory(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT ST_AsText(trajectory(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT ST_AsText(trajectory(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
SELECT ST_AsText(trajectory(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
-- 3D
SELECT ST_AsText(trajectory(tgeompoint 'Point(1 1 1)@2000-01-01'));
SELECT ST_AsText(trajectory(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'));
SELECT ST_AsText(trajectory(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT ST_AsText(trajectory(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01'));
SELECT ST_AsText(trajectory(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'));
SELECT ST_AsText(trajectory(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'));

SELECT ST_AsText(trajectory(tgeompoint '{[Point(1 1)@2001-01-01], [Point(1 1)@2001-02-01], [Point(1 1)@2001-03-01]}'));
SELECT ST_AsText(trajectory(tgeogpoint '{[Point(1 1)@2001-01-01], [Point(1 1)@2001-02-01], [Point(1 1)@2001-03-01]}'));
SELECT ST_AsText(trajectory(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02],[Point(1 1)@2000-01-03, Point(2 2)@2000-01-04]}'));

--------------------------------------------------------

-- 2D
SELECT round(length(tgeompoint 'Point(1 1)@2000-01-01')::numeric, 6);
SELECT round(length(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}')::numeric, 6);
SELECT round(length(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeogpoint 'Point(1.5 1.5)@2000-01-01')::numeric, 6);
SELECT round(length(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}')::numeric, 6);
SELECT round(length(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);
-- 3D
SELECT round(length(tgeompoint 'Point(1 1 1)@2000-01-01')::numeric, 6);
SELECT round(length(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}')::numeric, 6);
SELECT round(length(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01')::numeric, 6);
SELECT round(length(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}')::numeric, 6);
SELECT round(length(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);

-- 2D
SELECT round(cumulativeLength(tgeompoint 'Point(1 1)@2000-01-01'), 6);
SELECT round(cumulativeLength(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'), 6);
SELECT round(cumulativeLength(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Point(1.5 1.5)@2000-01-01'), 6);
SELECT round(cumulativeLength(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'), 6);
SELECT round(cumulativeLength(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'), 6);
-- 3D
SELECT round(cumulativeLength(tgeompoint 'Point(1 1 1)@2000-01-01'), 6);
SELECT round(cumulativeLength(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'), 6);
SELECT round(cumulativeLength(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01'), 6);
SELECT round(cumulativeLength(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'), 6);
SELECT round(cumulativeLength(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'), 6);

-- 2D
SELECT round(speed(tgeompoint 'Point(1 1)@2000-01-01'), 6);
SELECT round(speed(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'), 6);
SELECT round(speed(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round(speed(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round(speed(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round(speed(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round(speed(tgeogpoint 'Point(1.5 1.5)@2000-01-01'), 6);
SELECT round(speed(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'), 6);
SELECT round(speed(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'), 6);
SELECT round(speed(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'), 6);
SELECT round(speed(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'), 6);
SELECT round(speed(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'), 6);
-- 3D
SELECT round(speed(tgeompoint 'Point(1 1 1)@2000-01-01'), 6);
SELECT round(speed(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'), 6);
SELECT round(speed(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round(speed(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(speed(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round(speed(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(speed(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01'), 6);
SELECT round(speed(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'), 6);
SELECT round(speed(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'), 6);
SELECT round(speed(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'), 6);
SELECT round(speed(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'), 6);
SELECT round(speed(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'), 6);

-- 2D
SELECT st_astext(twcentroid(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT st_astext(twcentroid(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT st_astext(twcentroid(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT st_astext(twcentroid(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT st_astext(twcentroid(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT st_astext(twcentroid(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
-- 3D
SELECT st_astext(twcentroid(tgeompoint 'Point(1 1 1)@2000-01-01'));
SELECT st_astext(twcentroid(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'));
SELECT st_astext(twcentroid(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'));
SELECT st_astext(twcentroid(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT st_astext(twcentroid(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'));
SELECT st_astext(twcentroid(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));

-- 2D
SELECT round(degrees(azimuth(tgeompoint 'Point(1 1)@2000-01-01')), 6);
SELECT round(degrees(azimuth(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}')), 6);
SELECT round(degrees(azimuth(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')), 6);
SELECT round(degrees(azimuth(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')), 6);
SELECT round(degrees(azimuth(tgeogpoint 'Point(1.5 1.5)@2000-01-01')), 6);
SELECT round(degrees(azimuth(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}')), 6);
SELECT round(degrees(azimuth(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);
SELECT round(degrees(azimuth(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);
-- 3D
SELECT round(degrees(azimuth(tgeompoint 'Point(1 1 1)@2000-01-01')), 6);
SELECT round(degrees(azimuth(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}')), 6);
SELECT round(degrees(azimuth(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')), 6);
SELECT round(degrees(azimuth(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')), 6);
SELECT round(degrees(azimuth(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01')), 6);
SELECT round(degrees(azimuth(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}')), 6);
SELECT round(degrees(azimuth(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6);
SELECT round(degrees(azimuth(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6);

SELECT round(degrees(azimuth(tgeogpoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]}')), 6);
SELECT round(degrees(azimuth(tgeogpoint '{[Point(1 1)@2000-01-01], [Point(2 2)@2000-01-02], [Point(1 1)@2000-01-03]}')), 6);

SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02]')), 6);

SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]')), 6);
SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02)')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(1 1)@2000-01-02)')), 6);

SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02, Point(1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02, Point(1 1)@2000-01-03)')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(0 0)@2000-01-02, Point(1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(0 0)@2000-01-02, Point(1 1)@2000-01-03)')), 6);

SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03)')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03)')), 6);

SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03, Point(0 0)@2000-01-04]')), 6);
SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03, Point(0 0)@2000-01-04)')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03, Point(0 0)@2000-01-04]')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03, Point(0 0)@2000-01-04)')), 6);

--------------------------------------------------------

-- 2D
SELECT asText(atGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));
-- 3D
SELECT asText(atGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(atGeometry(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(atGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring Z empty'));
SELECT asText(atGeometry(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring Z empty'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty'));

SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01]', geometry 'Linestring(0 0,1 1)'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02]','Point(2 2)'));
SELECT asText(atGeometry(tgeompoint '[Point(0 1)@2000-01-01,Point(5 1)@2000-01-05]', geometry 'Linestring(0 0,2 2,3 1,4 1,5 0)'));

-- NULL
SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01]', geometry 'Linestring(2 2,3 3)'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]}', geometry 'Linestring(0 1,1 2)'));
SELECT asText(atGeometry(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02)', geometry 'Linestring(1 1,2 2)'));

/* Errors */
SELECT atGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT atGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');

-- 2D
SELECT asText(minusGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));
-- 3D
SELECT asText(minusGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(minusGeometry(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring Z empty'));
SELECT asText(minusGeometry(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring Z empty'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty'));

SELECT asText(minusGeometry(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02)', geometry 'Linestring(0 1,2 1)'));
SELECT asText(minusGeometry(tgeompoint '{[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02)}', geometry 'Linestring(0 1,2 1)'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02]','Point(2 2)'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02],[Point(3 3)@2000-01-03]}','Point(2 2)'));
/* Errors */
SELECT minusGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT minusGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');

--------------------------------------------------------

SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));

SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint 'Point(1.5 1.5)@2000-01-01', geography 'Linestring(0 0,3 3)'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', geography 'Linestring(0 0,3 3)'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', geography 'Linestring(0 0,3 3)'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', geography 'Linestring(0 0,3 3)'),6));
SELECT asText(NearestApproachInstant(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', geography 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', geography 'Linestring(0 0,3 3)'));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint 'Point(1.5 1.5)@2000-01-01', geography 'Linestring empty'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', geography 'Linestring empty'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', geography 'Linestring empty'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', geography 'Linestring empty'),6));
SELECT asText(NearestApproachInstant(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', geography 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', geography 'Linestring empty'));

SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(geography 'Linestring(0 0,3 3)', tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT asText(NearestApproachInstant(geography 'Linestring(0 0,3 3)', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asText(NearestApproachInstant(geography 'Linestring(0 0,3 3)', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asText(NearestApproachInstant(geography 'Linestring(0 0,3 3)', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));

SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint 'Point(2.5 2.5)@2000-01-01'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint 'Point(2.5 2.5)@2000-01-01'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint 'Point(2.5 2.5)@2000-01-01'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(2.5 2.5)@2000-01-01'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'),6));

SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'),6));
SELECT asText(setPrecision(NearestApproachInstant(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'),6));

/* Errors */
SELECT NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');
SELECT NearestApproachInstant(geometry 'SRID=5676;Linestring(1 1,2 2)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT NearestApproachInstant(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'SRID=5676;Point(1 1)@2000-01-01');
SELECT NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1 1)@2000-01-01');

--------------------------------------------------------

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)')::numeric, 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty')::numeric, 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring Z empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring Z empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty')::numeric, 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(1.5 1.5)@2000-01-01', geography 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', geography 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', geography 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', geography 'Linestring(0 0,3 3)')::numeric, 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(1.5 1.5)@2000-01-01', geography 'Linestring empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', geography 'Linestring empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', geography 'Linestring empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', geography 'Linestring empty')::numeric, 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', geography 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', geography 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', geography 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', geography 'Linestring(0 0 0,3 3 3)')::numeric, 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', geography 'Linestring Z empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', geography 'Linestring Z empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', geography 'Linestring Z empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', geography 'Linestring Z empty')::numeric, 6);

SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeompoint 'Point(1 1)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeompoint 'Point(1 1)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint 'Point(1 1 1)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeompoint 'Point(1 1 1)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(geography 'Linestring(0 0,3 3)', tgeogpoint 'Point(1.5 1.5)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring(0 0,3 3)', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring(0 0,3 3)', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring(0 0,3 3)', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeogpoint 'Point(1.5 1.5)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(geography 'Linestring(0 0 0,3 3 3)', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring(0 0 0,3 3 3)', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring(0 0 0,3 3 3)', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring(0 0 0,3 3 3)', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint 'Point(2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint 'Point(2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint 'Point(2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]'::numeric, 6));
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);

SELECT round((tgeompoint 'Point(1 1)@2000-01-01' |=| geometry 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round((tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| geometry 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round((tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| geometry 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round((tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| geometry 'Linestring(0 0,3 3)')::numeric, 6);

SELECT round((tgeompoint 'Point(1 1)@2000-01-01' |=| geometry 'Linestring empty')::numeric, 6);
SELECT round((tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| geometry 'Linestring empty')::numeric, 6);
SELECT round((tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| geometry 'Linestring empty')::numeric, 6);
SELECT round((tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| geometry 'Linestring empty')::numeric, 6);

SELECT round((tgeompoint 'Point(1 1 1)@2000-01-01' |=| geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round((tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round((tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);

SELECT round((tgeompoint 'Point(1 1 1)@2000-01-01' |=| geometry 'Linestring Z empty')::numeric, 6);
SELECT round((tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| geometry 'Linestring Z empty')::numeric, 6);
SELECT round((tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| geometry 'Linestring Z empty')::numeric, 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| geometry 'Linestring Z empty')::numeric, 6);

SELECT round((tgeogpoint 'Point(1.5 1.5)@2000-01-01' |=| geography 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round((tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' |=| geography 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round((tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' |=| geography 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round((tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' |=| geography 'Linestring(0 0,3 3)')::numeric, 6);

SELECT round((tgeogpoint 'Point(1.5 1.5)@2000-01-01' |=| geography 'Linestring empty')::numeric, 6);
SELECT round((tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' |=| geography 'Linestring empty')::numeric, 6);
SELECT round((tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' |=| geography 'Linestring empty')::numeric, 6);
SELECT round((tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' |=| geography 'Linestring empty')::numeric, 6);

SELECT round((tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' |=| geography 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round((tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' |=| geography 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round((tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' |=| geography 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round((tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' |=| geography 'Linestring(0 0 0,3 3 3)')::numeric, 6);

SELECT round((tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' |=| geography 'Linestring Z empty')::numeric, 6);
SELECT round((tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' |=| geography 'Linestring Z empty')::numeric, 6);
SELECT round((tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' |=| geography 'Linestring Z empty')::numeric, 6);
SELECT round((tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' |=| geography 'Linestring Z empty')::numeric, 6);

SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeompoint 'Point(1 1)@2000-01-01')::numeric, 6);
SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}')::numeric, 6);
SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')::numeric, 6);
SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);

SELECT round((geometry 'Linestring empty' |=| tgeompoint 'Point(1 1)@2000-01-01')::numeric, 6);
SELECT round((geometry 'Linestring empty' |=| tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}')::numeric, 6);
SELECT round((geometry 'Linestring empty' |=| tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')::numeric, 6);
SELECT round((geometry 'Linestring empty' |=| tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);

SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeompoint 'Point(1 1 1)@2000-01-01')::numeric, 6);
SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}')::numeric, 6);
SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')::numeric, 6);
SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);

SELECT round((geometry 'Linestring Z empty' |=| tgeompoint 'Point(1 1 1)@2000-01-01')::numeric, 6);
SELECT round((geometry 'Linestring Z empty' |=| tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}')::numeric, 6);
SELECT round((geometry 'Linestring Z empty' |=| tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')::numeric, 6);
SELECT round((geometry 'Linestring Z empty' |=| tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);

SELECT round((geography 'Linestring(0 0,3 3)' |=| tgeogpoint 'Point(1.5 1.5)@2000-01-01')::numeric, 6);
SELECT round((geography 'Linestring(0 0,3 3)' |=| tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}')::numeric, 6);
SELECT round((geography 'Linestring(0 0,3 3)' |=| tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round((geography 'Linestring(0 0,3 3)' |=| tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);

SELECT round((geography 'Linestring empty' |=| tgeogpoint 'Point(1.5 1.5)@2000-01-01')::numeric, 6);
SELECT round((geography 'Linestring empty' |=| tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}')::numeric, 6);
SELECT round((geography 'Linestring empty' |=| tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round((geography 'Linestring empty' |=| tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);

SELECT round((geography 'Linestring(0 0 0,3 3 3)' |=| tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01')::numeric, 6);
SELECT round((geography 'Linestring(0 0 0,3 3 3)' |=| tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}')::numeric, 6);
SELECT round((geography 'Linestring(0 0 0,3 3 3)' |=| tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round((geography 'Linestring(0 0 0,3 3 3)' |=| tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);

SELECT round((geography 'Linestring Z empty' |=| tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01')::numeric, 6);
SELECT round((geography 'Linestring Z empty' |=| tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}')::numeric, 6);
SELECT round((geography 'Linestring Z empty' |=| tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round((geography 'Linestring Z empty' |=| tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);

SELECT round((tgeompoint 'Point(1 1)@2000-01-01' |=| tgeompoint 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| tgeompoint 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| tgeompoint 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| tgeompoint 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeompoint 'Point(1 1)@2000-01-01' |=| tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeompoint 'Point(1 1)@2000-01-01' |=| tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeompoint 'Point(1 1)@2000-01-01' |=| tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round((tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round((tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round((tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);

SELECT round((tgeompoint 'Point(1 1 1)@2000-01-01' |=| tgeompoint 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| tgeompoint 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| tgeompoint 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| tgeompoint 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeompoint 'Point(1 1 1)@2000-01-01' |=| tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeompoint 'Point(1 1 1)@2000-01-01' |=| tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeompoint 'Point(1 1 1)@2000-01-01' |=| tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round((tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round((tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);

SELECT round((tgeogpoint 'Point(1.5 1.5)@2000-01-01' |=| tgeogpoint 'Point(2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round((tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' |=| tgeogpoint 'Point(2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round((tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' |=| tgeogpoint 'Point(2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round((tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' |=| tgeogpoint 'Point(2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round((tgeogpoint 'Point(1.5 1.5)@2000-01-01' |=| tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round((tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' |=| tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round((tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' |=| tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round((tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' |=| tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round((tgeogpoint 'Point(1.5 1.5)@2000-01-01' |=| tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round((tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' |=| tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round((tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' |=| tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round((tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' |=| tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round((tgeogpoint 'Point(1.5 1.5)@2000-01-01' |=| tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round((tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' |=| tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round((tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' |=| tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round((tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' |=| tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);

SELECT round((tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' |=| tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round((tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' |=| tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round((tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' |=| tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round((tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' |=| tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')::numeric, 6);
SELECT round((tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' |=| tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round((tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' |=| tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round((tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' |=| tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round((tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' |=| tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')::numeric, 6);
SELECT round((tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' |=| tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round((tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' |=| tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round((tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' |=| tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round((tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' |=| tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')::numeric, 6);
SELECT round((tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' |=| tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round((tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' |=| tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round((tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' |=| tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round((tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' |=| tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);

/* Errors */
SELECT NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');
SELECT NearestApproachDistance(geometry 'SRID=5676;Linestring(1 1,2 2)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT NearestApproachDistance(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'SRID=5676;Point(1 1)@2000-01-01');
SELECT NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1 1)@2000-01-01');

--------------------------------------------------------

SELECT ST_AsTexT(ShortestLine(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));

SELECT ST_AsTexT(ShortestLine(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));

SELECT ST_AsTexT(ShortestLine(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'));

SELECT ST_AsTexT(ShortestLine(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring Z empty'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring Z empty'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty'));

SELECT ST_AsTexT(ShortestLine(tgeogpoint 'Point(1.5 1.5)@2000-01-01', geography 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', geography 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', geography 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', geography 'Linestring(0 0,3 3)'));

SELECT ST_AsTexT(ShortestLine(tgeogpoint 'Point(1.5 1.5)@2000-01-01', geography 'Linestring empty'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', geography 'Linestring empty'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', geography 'Linestring empty'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', geography 'Linestring empty'));

SELECT ST_AsTexT(ShortestLine(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', geography 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', geography 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', geography 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', geography 'Linestring(0 0 0,3 3 3)'));

SELECT ST_AsTexT(ShortestLine(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', geography 'Linestring Z empty'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', geography 'Linestring Z empty'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', geography 'Linestring Z empty'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', geography 'Linestring Z empty'));

SELECT ST_AsTexT(ShortestLine(geometry 'Linestring(0 0,3 3)', tgeompoint 'Point(1 1)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(geometry 'Linestring(0 0,3 3)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(geometry 'Linestring(0 0,3 3)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(geometry 'Linestring(0 0,3 3)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT ST_AsTexT(ShortestLine(geometry 'Linestring empty', tgeompoint 'Point(1 1)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(geometry 'Linestring empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(geometry 'Linestring empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(geometry 'Linestring empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT ST_AsTexT(ShortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint 'Point(1 1 1)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));

SELECT ST_AsTexT(ShortestLine(geometry 'Linestring Z empty', tgeompoint 'Point(1 1 1)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(geometry 'Linestring Z empty', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(geometry 'Linestring Z empty', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(geometry 'Linestring Z empty', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));

SELECT ST_AsTexT(ShortestLine(geography 'Linestring(0 0,3 3)', tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(geography 'Linestring(0 0,3 3)', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(geography 'Linestring(0 0,3 3)', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(geography 'Linestring(0 0,3 3)', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT ST_AsTexT(ShortestLine(geography 'Linestring empty', tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(geography 'Linestring empty', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(geography 'Linestring empty', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(geography 'Linestring empty', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT ST_AsTexT(ShortestLine(geography 'Linestring(0 0 0,3 3 3)', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(geography 'Linestring(0 0 0,3 3 3)', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(geography 'Linestring(0 0 0,3 3 3)', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(geography 'Linestring(0 0 0,3 3 3)', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'));

SELECT ST_AsTexT(ShortestLine(geography 'Linestring Z empty', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(geography 'Linestring Z empty', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(geography 'Linestring Z empty', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(geography 'Linestring Z empty', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'));

SELECT ST_AsTexT(ShortestLine(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT ST_AsTexT(ShortestLine(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT ST_AsTexT(ShortestLine(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));

SELECT ST_AsTexT(ShortestLine(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint 'Point(2.5 2.5)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint 'Point(2.5 2.5)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint 'Point(2.5 2.5)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(2.5 2.5)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT ST_AsTexT(ShortestLine(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'));
SELECT ST_AsTexT(ShortestLine(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'));

SELECT ST_AsTexT(ShortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02], (Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]}', tgeompoint '{[Point(3 3)@2000-01-01, Point(3 3)@2000-01-02], (Point(2 2)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsText(shortestLine(tgeompoint 'Interp=Stepwise;[Point(0 0)@2000-01-01, Point(2 0)@2000-01-02]','[Point(1 1)@2000-01-01,Point(1 1)@2000-01-02]'));
SELECT ST_AsText(shortestLine(tgeompoint 'Interp=Stepwise;{(Point(0 0)@2000-01-01, Point(2 0)@2000-01-02]}','{[Point(1 1)@2000-01-01,Point(1 1)@2000-01-02]}'));
SELECT ST_AsText(shortestLine(tgeompoint '{[Point(0 0)@2000-01-01, Point(2 0)@2000-01-02]}','{(Point(1 1)@2000-01-01,Point(2 2)@2000-01-02]}'));
SELECT ST_AsText(shortestLine(tgeompoint '{[Point(0 0)@2000-01-01],(Point(0 0)@2000-01-02, Point(1 1)@2000-01-03)}','{[Point(1 3)@2000-01-01, Point(1 2)@2000-01-03]}'));
SELECT ST_AsText(shortestLine(tgeompoint '{[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02),[Point(0 0)@2000-01-03]}','{[Point(1 3)@2000-01-01, Point(1 2)@2000-01-03]}'));
SELECT ST_AsText(shortestLine(tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02)', '[Point(4 1)@2000-01-01, Point(2 1)@2000-01-02)'));
-- NULL
SELECT shortestline(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02),[Point(1 1)@2001-01-03, Point(2 2)@2001-01-04]}', tgeompoint '[Point(1 1)@2001-01-02, Point(1 1)@2001-01-03)');
SELECT shortestline(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02),[Point(1 1)@2001-01-03, Point(2 2)@2001-01-04]}', tgeompoint '{[Point(1 1)@2001-01-02, Point(1 1)@2001-01-03)}');
/* Errors */
SELECT shortestLine(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT shortestLine(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');
SELECT shortestLine(geometry 'SRID=5676;Linestring(1 1,2 2)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT shortestLine(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT shortestLine(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'SRID=5676;Point(1 1)@2000-01-01');
SELECT shortestLine(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1 1)@2000-01-01');

--------------------------------------------------------

SELECT ST_AsText(tgeompoint 'Point(1 1)@2000-01-01'::geometry);
SELECT ST_AsText(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'::geometry);
SELECT ST_AsText(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'::geometry);
SELECT ST_AsText(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'::geometry);
SELECT ST_AsText(tgeogpoint 'Point(1.5 1.5)@2000-01-01'::geography);
SELECT ST_AsText(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'::geography);
SELECT ST_AsText(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'::geography);
SELECT ST_AsText(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'::geography);

SELECT ST_AsText(tgeompoint 'Point(1 1 1)@2000-01-01'::geometry);
SELECT ST_AsText(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'::geometry);
SELECT ST_AsText(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'::geometry);
SELECT ST_AsText(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'::geometry);
SELECT ST_AsText(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01'::geography);
SELECT ST_AsText(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'::geography);
SELECT ST_AsText(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'::geography);
SELECT ST_AsText(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'::geography);

SELECT ST_AsText(tgeompoint 'Interp=Stepwise;[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]'::geometry);

--------------------------------------------------------

SELECT asText((tgeompoint 'Point(1 1)@2000-01-01'::geometry)::tgeompoint);
SELECT asText((tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'::geometry)::tgeompoint);
SELECT asText((tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'::geometry)::tgeompoint);
SELECT asText((tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'::geometry)::tgeompoint);
SELECT asText((tgeogpoint 'Point(1.5 1.5)@2000-01-01'::geography)::tgeogpoint);
SELECT asText((tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'::geography)::tgeogpoint);
SELECT asText((tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'::geography)::tgeogpoint);
SELECT asText((tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'::geography)::tgeogpoint);

SELECT asText((tgeompoint 'Point(1 1 1)@2000-01-01'::geometry)::tgeompoint);
SELECT asText((tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'::geometry)::tgeompoint);
SELECT asText((tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'::geometry)::tgeompoint);
SELECT asText((tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'::geometry)::tgeompoint);
SELECT asText((tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01'::geography)::tgeogpoint);
SELECT asText((tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'::geography)::tgeogpoint);
SELECT asText((tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'::geography)::tgeogpoint);
SELECT asText((tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'::geography)::tgeogpoint);

-------------------------------------------------------------------------------

/* Errors */
SELECT geometry 'POINT empty'::tgeompoint;
SELECT geometry 'POINT(1 1)'::tgeompoint;
SELECT geometry 'POLYGON M((1 1 946681200,1 2 946681200,2 2 946681200,2 1 946681200,1 1 946681200))'::tgeompoint;
SELECT geometry 'MULTIPOINT M (1 1 946767600,1 1 946681200)'::tgeompoint;
SELECT geometry 'LINESTRING M (1 1 946767600,1 1 946681200)'::tgeompoint;
SELECT geometry 'GEOMETRYCOLLECTION M (LINESTRING M (1 1 946681200,2 2 946767600),
POLYGON M((1 1 946681200,1 2 946681200,2 2 946681200,2 1 946681200,1 1 946681200)))'::tgeompoint;

-------------------------------------------------------------------------------

SELECT ST_AsText(geoMeasure(tgeompoint 'Point(1 1)@2000-01-01', '5@2000-01-01'));
SELECT ST_AsText(geoMeasure(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '{5@2000-01-01, 7@2000-01-02, 5@2000-01-03}'));
SELECT ST_AsText(geoMeasure(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[5@2000-01-01, 7@2000-01-02, 5@2000-01-03]'));
SELECT ST_AsText(geoMeasure(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', '{[5@2000-01-01, 7@2000-01-02, 5@2000-01-03],[5@2000-01-04, 5@2000-01-05]}'));
SELECT ST_AsText(geoMeasure(tgeogpoint 'Point(1.5 1.5)@2000-01-01', '5.5@2000-01-01'));
SELECT ST_AsText(geoMeasure(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', '{5.5@2000-01-01, 7.5@2000-01-02, 5@2000-01-03}'));
SELECT ST_AsText(geoMeasure(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', '[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03]'));
SELECT ST_AsText(geoMeasure(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', '[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03]', true));
SELECT ST_AsText(geoMeasure(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', '{[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03],[5.5@2000-01-04, 5.5@2000-01-05]}'));
SELECT ST_AsText(geoMeasure(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', '{[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03],[5.5@2000-01-04, 5.5@2000-01-05]}', true));

SELECT ST_AsText(geoMeasure(tgeompoint 'Point(1 1 1)@2000-01-01', '5@2000-01-01'));
SELECT ST_AsText(geoMeasure(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', '{5@2000-01-01, 7@2000-01-02, 5@2000-01-03}'));
SELECT ST_AsText(geoMeasure(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', '[5@2000-01-01, 7@2000-01-02, 5@2000-01-03]'));
SELECT ST_AsText(geoMeasure(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', '{[5@2000-01-01, 7@2000-01-02, 5@2000-01-03],[5@2000-01-04, 5@2000-01-05]}'));
SELECT ST_AsText(geoMeasure(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', '5.5@2000-01-01'));
SELECT ST_AsText(geoMeasure(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', '{5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03}'));
SELECT ST_AsText(geoMeasure(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', '[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03]'));
SELECT ST_AsText(geoMeasure(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', '{[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03],[5.5@2000-01-04, 5.5@2000-01-05]}'));

SELECT ST_AsText(geoMeasure(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[5@2000-01-01, 7@2000-01-02, 5@2000-01-03]'));
SELECT ST_AsText(geoMeasure(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', '[5@2000-01-01, 7@2000-01-02]'));
SELECT ST_AsText(geoMeasure(tgeompoint '[Point(1 1)@2000-01-01]', '[5@2000-01-01]', true));
SELECT ST_AsText(geoMeasure(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[5@2000-01-01, 7@2000-01-02, 5@2000-01-03]', true));
SELECT ST_AsText(geoMeasure(tgeompoint '{[Point(1 1)@2000-01-01]}', '{[5@2000-01-01]}', true));
SELECT ST_AsText(geoMeasure(tgeompoint '{[Point(1 1)@2000-01-01],[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]}', '{[5@2000-01-01],[6@2000-01-02, 7@2000-01-03]}', true));

-- NULL
SELECT ST_AsText(geoMeasure(tgeompoint 'Point(1 1)@2000-01-01', '5@2000-01-02'));

-------------------------------------------------------------------------------

-- Only distance specified
SELECT ST_AsText(trajectory(simplify(tgeompoint '[Point(0 4)@2000-01-01,
	Point(1 1)@2000-01-02, Point(2 3)@2000-01-03, Point(3 1)@2000-01-04,
	Point(4 3)@2000-01-05, Point(5 0)@2000-01-06, Point(6 4)@2000-01-07]', 1.5)));
SELECT ST_AsText(trajectory(simplify(tgeompoint '[Point(0 4)@2000-01-01,
	Point(1 1)@2000-01-02, Point(2 3)@2000-01-03, Point(3 1)@2000-01-04,
	Point(4 3)@2000-01-05, Point(5 0)@2000-01-06, Point(6 4)@2000-01-07]', 4)));
-- Both distance and delta speed specified
SELECT ST_AsText(trajectory(simplify(tgeompoint '[Point(0 4)@2000-01-01,
	Point(1 1)@2000-01-02, Point(2 3)@2000-01-03, Point(3 1)@2000-01-04,
	Point(4 3)@2000-01-05, Point(5 0)@2000-01-06, Point(6 4)@2000-01-07]', 4, 1 / 1e5)));
SELECT ST_AsText(trajectory(simplify(tgeompoint '[Point(0 4)@2000-01-01,
	Point(1 1)@2000-01-02, Point(2 3)@2000-01-03, Point(3 1)@2000-01-04,
	Point(4 3)@2000-01-05, Point(5 0)@2000-01-06, Point(6 4)@2000-01-07]', 4, 3 / 1e5)));

SELECT ST_AsText(trajectory(simplify(tgeompoint '{[Point(0 4)@2000-01-01,
	Point(1 1)@2000-01-02, Point(2 3)@2000-01-03, Point(3 1)@2000-01-04,
	Point(4 3)@2000-01-05, Point(5 0)@2000-01-06, Point(6 4)@2000-01-07]}', 4)));
SELECT ST_AsText(trajectory(simplify(tgeompoint '{[Point(0 4)@2000-01-01,
	Point(1 1)@2000-01-02, Point(2 3)@2000-01-03, Point(3 1)@2000-01-04],
	[Point(4 3)@2000-01-05, Point(5 0)@2000-01-06, Point(6 4)@2000-01-07]}', 4)));
SELECT ST_AsText(trajectory(simplify(tgeompoint '{[Point(0 4)@2000-01-01,
	Point(1 1)@2000-01-02, Point(2 3)@2000-01-03, Point(3 1)@2000-01-04],
	[Point(4 3)@2000-01-05, Point(5 0)@2000-01-06, Point(6 4)@2000-01-07],[Point(7 4)@2000-01-08]}', 4)));

-- Only distance specified
SELECT ST_AsText(trajectory(simplify(tgeompoint '[Point(0 4 0)@2000-01-01,
	Point(1 1 1)@2000-01-02, Point(2 3 2)@2000-01-03, Point(3 1 3)@2000-01-04,
	Point(4 3 4)@2000-01-05, Point(5 0 5)@2000-01-06, Point(6 4 6)@2000-01-07]', 1.5)));
SELECT ST_AsText(trajectory(simplify(tgeompoint '[Point(0 4 0)@2000-01-01,
	Point(1 1 1)@2000-01-02, Point(2 3 2)@2000-01-03, Point(3 1 3)@2000-01-04,
	Point(4 3 4)@2000-01-05, Point(5 0 5)@2000-01-06, Point(6 4 6)@2000-01-07]', 4)));
-- Both distance and delta speed specified
SELECT ST_AsText(trajectory(simplify(tgeompoint '[Point(0 4 0)@2000-01-01,
	Point(1 1 1)@2000-01-02, Point(2 3 2)@2000-01-03, Point(3 1 3)@2000-01-04,
	Point(4 3 4)@2000-01-05, Point(5 0 5)@2000-01-06, Point(6 4 6)@2000-01-07]', 4, 1 / 1e5)));
SELECT ST_AsText(trajectory(simplify(tgeompoint '[Point(0 4 0)@2000-01-01,
	Point(1 1 1)@2000-01-02, Point(2 3 2)@2000-01-03, Point(3 1 3)@2000-01-04,
	Point(4 3 4)@2000-01-05, Point(5 0 5)@2000-01-06, Point(6 4 6)@2000-01-07]', 4, 3 / 1e5)));

-- No simplification, return a copy of the original temporal point
SELECT asText(simplify(tgeompoint 'Point(0 4)@2000-01-01', 1.5));
SELECT asText(simplify(tgeompoint '{Point(0 4)@2000-01-01, Point(1 1)@2000-01-02}', 1.5));
SELECT asText(simplify(tgeompoint '[Point(0 4)@2000-01-01, Point(1 1)@2000-01-02]', 1.5));
SELECT asText(simplify(tgeompoint 'Interp=Stepwise;[Point(0 4)@2000-01-01, Point(1 1)@2000-01-02, Point(2 3)@2000-01-03]', 1.5));

-- Big temporal point > 256 instants
SELECT asText(simplify(tgeompoint '[POINT(77 69)@2000-01-02, POINT(83 75)@2000-01-03, POINT(85 77)@2000-01-04, POINT(82 73)@2000-01-05, POINT(77 69)@2000-01-06, POINT(78 70)@2000-01-07, POINT(73 65)@2000-01-08, POINT(75 67)@2000-01-09, POINT(69 61)@2000-01-10, POINT(62 54)@2000-01-11, POINT(54 46)@2000-01-12, POINT(49 41)@2000-01-13, POINT(57 48)@2000-01-14, POINT(49 41)@2000-01-15, POINT(52 44)@2000-01-16, POINT(56 48)@2000-01-17, POINT(50 41)@2000-01-18, POINT(41 33)@2000-01-19, POINT(45 37)@2000-01-20, POINT(50 42)@2000-01-21, POINT(49 41)@2000-01-22, POINT(55 47)@2000-01-23, POINT(54 46)@2000-01-24, POINT(60 52)@2000-01-25, POINT(58 50)@2000-01-26, POINT(58 50)@2000-01-27, POINT(56 48)@2000-01-28, POINT(62 53)@2000-01-29, POINT(64 55)@2000-01-30, POINT(56 47)@2000-01-31, POINT(53 45)@2000-02-01, POINT(54 45)@2000-02-02, POINT(61 53)@2000-02-03, POINT(71 63)@2000-02-04, POINT(78 70)@2000-02-05, POINT(71 63)@2000-02-06, POINT(72 63)@2000-02-07, POINT(64 56)@2000-02-08, POINT(69 60)@2000-02-09, POINT(73 65)@2000-02-10, POINT(69 61)@2000-02-11, POINT(76 68)@2000-02-12, POINT(85 76)@2000-02-13, POINT(78 70)@2000-02-14, POINT(87 79)@2000-02-15, POINT(89 81)@2000-02-16, POINT(97 88)@2000-02-17, POINT(89 81)@2000-02-18, POINT(93 85)@2000-02-19, POINT(94 86)@2000-02-20, POINT(87 94)@2000-02-21, POINT(80 87)@2000-02-22, POINT(77 84)@2000-02-23, POINT(74 80)@2000-02-24, POINT(83 89)@2000-02-25, POINT(88 95)@2000-02-26, POINT(95 89)@2000-02-27, POINT(92 86)@2000-02-28, POINT(93 87)@2000-02-29, POINT(91 85)@2000-03-01, POINT(90 84)@2000-03-02, POINT(98 92)@2000-03-03, POINT(89 83)@2000-03-04, POINT(86 80)@2000-03-05, POINT(94 88)@2000-03-06, POINT(100 94)@2000-03-07, POINT(100 94)@2000-03-08, POINT(98 92)@2000-03-09, POINT(89 83)@2000-03-10, POINT(84 78)@2000-03-11, POINT(76 70)@2000-03-12, POINT(71 65)@2000-03-13, POINT(62 56)@2000-03-14, POINT(54 48)@2000-03-15, POINT(52 46)@2000-03-16, POINT(42 36)@2000-03-17, POINT(45 40)@2000-03-18, POINT(41 35)@2000-03-19, POINT(34 28)@2000-03-20, POINT(31 25)@2000-03-21, POINT(38 32)@2000-03-22, POINT(28 22)@2000-03-23, POINT(28 22)@2000-03-24, POINT(23 17)@2000-03-25, POINT(20 14)@2000-03-26, POINT(18 13)@2000-03-27, POINT(8 3)@2000-03-28, POINT(2 9)@2000-03-29, POINT(8 15)@2000-03-30, POINT(9 16)@2000-03-31, POINT(10 18)@2000-04-01, POINT(5 13)@2000-04-02, POINT(4 12)@2000-04-03, POINT(5 12)@2000-04-04, POINT(6 14)@2000-04-05, POINT(3 11)@2000-04-06, POINT(7 7)@2000-04-07, POINT(15 16)@2000-04-08, POINT(20 21)@2000-04-09, POINT(15 16)@2000-04-10, POINT(11 12)@2000-04-11, POINT(19 20)@2000-04-12, POINT(18 19)@2000-04-13, POINT(16 17)@2000-04-14, POINT(25 26)@2000-04-15, POINT(32 33)@2000-04-16, POINT(30 31)@2000-04-17, POINT(33 34)@2000-04-18, POINT(26 27)@2000-04-19, POINT(27 28)@2000-04-20, POINT(37 38)@2000-04-21, POINT(46 47)@2000-04-22, POINT(48 49)@2000-04-23, POINT(48 49)@2000-04-24, POINT(42 43)@2000-04-25, POINT(50 51)@2000-04-26, POINT(59 60)@2000-04-27, POINT(53 54)@2000-04-28, POINT(44 45)@2000-04-29, POINT(54 55)@2000-05-01, POINT(57 58)@2000-05-02, POINT(67 68)@2000-05-03, POINT(61 62)@2000-05-04, POINT(54 55)@2000-05-05, POINT(56 57)@2000-05-06, POINT(57 58)@2000-05-07, POINT(57 58)@2000-05-08, POINT(60 61)@2000-05-09, POINT(56 57)@2000-05-10, POINT(61 62)@2000-05-11, POINT(71 71)@2000-05-12, POINT(64 65)@2000-05-13, POINT(59 59)@2000-05-14, POINT(55 56)@2000-05-15, POINT(48 49)@2000-05-16, POINT(40 41)@2000-05-17, POINT(50 51)@2000-05-19, POINT(46 46)@2000-05-20, POINT(41 42)@2000-05-21, POINT(46 47)@2000-05-22, POINT(41 42)@2000-05-23, POINT(48 49)@2000-05-24, POINT(43 44)@2000-05-25, POINT(42 43)@2000-05-26, POINT(47 48)@2000-05-27, POINT(41 42)@2000-05-28, POINT(45 45)@2000-05-29, POINT(51 52)@2000-05-30, POINT(60 61)@2000-05-31, POINT(58 59)@2000-06-01, POINT(58 58)@2000-06-02, POINT(66 67)@2000-06-03, POINT(68 69)@2000-06-04, POINT(71 72)@2000-06-05, POINT(71 72)@2000-06-06, POINT(57 58)@2000-06-08, POINT(51 52)@2000-06-09, POINT(49 50)@2000-06-10, POINT(58 58)@2000-06-11, POINT(51 51)@2000-06-12, POINT(52 53)@2000-06-13, POINT(45 46)@2000-06-14, POINT(45 46)@2000-06-15, POINT(50 51)@2000-06-16, POINT(45 46)@2000-06-17, POINT(39 40)@2000-06-18, POINT(39 40)@2000-06-19, POINT(40 41)@2000-06-20, POINT(40 40)@2000-06-21, POINT(35 36)@2000-06-22, POINT(40 41)@2000-06-23, POINT(37 38)@2000-06-24, POINT(38 38)@2000-06-25, POINT(32 33)@2000-06-26, POINT(23 24)@2000-06-27, POINT(28 29)@2000-06-28, POINT(44 45)@2000-06-30, POINT(47 48)@2000-07-01, POINT(43 44)@2000-07-02, POINT(40 41)@2000-07-03, POINT(43 44)@2000-07-04, POINT(50 51)@2000-07-05, POINT(41 42)@2000-07-06, POINT(33 34)@2000-07-07, POINT(24 25)@2000-07-08, POINT(17 18)@2000-07-09, POINT(13 14)@2000-07-10, POINT(12 13)@2000-07-11, POINT(4 5)@2000-07-12, POINT(3 4)@2000-07-13, POINT(12 13)@2000-07-14, POINT(7 8)@2000-07-15, POINT(16 17)@2000-07-16, POINT(21 22)@2000-07-17, POINT(22 22)@2000-07-18, POINT(14 15)@2000-07-19, POINT(10 11)@2000-07-20, POINT(1 2)@2000-07-21, POINT(3 4)@2000-07-22, POINT(4 5)@2000-07-23, POINT(10 11)@2000-07-24, POINT(19 20)@2000-07-25, POINT(11 12)@2000-07-26, POINT(2 2)@2000-07-27, POINT(11 12)@2000-07-28, POINT(18 19)@2000-07-29, POINT(34 35)@2000-07-31, POINT(34 35)@2000-08-01, POINT(28 29)@2000-08-02, POINT(24 25)@2000-08-03, POINT(8 9)@2000-08-05, POINT(4 5)@2000-08-06, POINT(10 10)@2000-08-07, POINT(2 3)@2000-08-08, POINT(2 3)@2000-08-10, POINT(3 4)@2000-08-11, POINT(5 6)@2000-08-12, POINT(15 15)@2000-08-13, POINT(17 17)@2000-08-14, POINT(24 24)@2000-08-15, POINT(31 32)@2000-08-16, POINT(29 30)@2000-08-17, POINT(26 27)@2000-08-18, POINT(17 18)@2000-08-19, POINT(19 20)@2000-08-20, POINT(18 19)@2000-08-21, POINT(21 22)@2000-08-22, POINT(14 15)@2000-08-23, POINT(9 10)@2000-08-24, POINT(11 12)@2000-08-25, POINT(6 7)@2000-08-26, POINT(2 3)@2000-08-27, POINT(4 5)@2000-08-28, POINT(13 14)@2000-08-29, POINT(7 8)@2000-08-30, POINT(7 8)@2000-08-31, POINT(9 10)@2000-09-01, POINT(6 7)@2000-09-02, POINT(13 14)@2000-09-03, POINT(16 17)@2000-09-04, POINT(16 17)@2000-09-05, POINT(9 9)@2000-09-06, POINT(17 18)@2000-09-07, POINT(18 19)@2000-09-08, POINT(21 22)@2000-09-09, POINT(20 20)@2000-09-10, POINT(12 13)@2000-09-11, POINT(7 8)@2000-09-12, POINT(5 6)@2000-09-13, POINT(10 10)@2000-09-14, POINT(1 2)@2000-09-15, POINT(6 7)@2000-09-16, POINT(14 14)@2000-09-17, POINT(13 14)@2000-09-18, POINT(9 10)@2000-09-19, POINT(14 15)@2000-09-20, POINT(21 22)@2000-09-21, POINT(31 31)@2000-09-22, POINT(39 40)@2000-09-23, POINT(31 32)@2000-09-24, POINT(32 33)@2000-09-25, POINT(25 26)@2000-09-26, POINT(23 24)@2000-09-27, POINT(11 12)@2000-09-29, POINT(13 14)@2000-09-30, POINT(23 24)@2000-10-02, POINT(33 34)@2000-10-03, POINT(34 35)@2000-10-04, POINT(32 33)@2000-10-06, POINT(36 36)@2000-10-07, POINT(33 34)@2000-10-08, POINT(23 24)@2000-10-09, POINT(20 21)@2000-10-10, POINT(26 27)@2000-10-11, POINT(19 20)@2000-10-12, POINT(20 21)@2000-10-13, POINT(14 15)@2000-10-14, POINT(22 22)@2000-10-15, POINT(25 26)@2000-10-16, POINT(24 24)@2000-10-17, POINT(14 15)@2000-10-18, POINT(6 7)@2000-10-19, POINT(16 17)@2000-10-21, POINT(26 27)@2000-10-22, POINT(30 31)@2000-10-23, POINT(33 34)@2000-10-24, POINT(25 26)@2000-10-25, POINT(21 22)@2000-10-26, POINT(27 28)@2000-10-27, POINT(27 28)@2000-10-28, POINT(27 27)@2000-10-29, POINT(17 18)@2000-10-30, POINT(9 10)@2000-10-31, POINT(3 4)@2000-11-01, POINT(9 10)@2000-11-02, POINT(0 1)@2000-11-03, POINT(5 6)@2000-11-04, POINT(0 1)@2000-11-05, POINT(1 2)@2000-11-06, POINT(2 0)@2000-11-07, POINT(5 3)@2000-11-08, POINT(6 3)@2000-11-09, POINT(11 9)@2000-11-10, POINT(9 7)@2000-11-11, POINT(13 11)@2000-11-12, POINT(9 7)@2000-11-13, POINT(13 11)@2000-11-15, POINT(22 20)@2000-11-16]', 10));

-------------------------------------------------------------------------------
