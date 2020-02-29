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

SELECT ST_AsTexT(ShortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02], (Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]}', 
tgeompoint '{[Point(3 3)@2000-01-01, Point(3 3)@2000-01-02], (Point(2 2)@2000-01-04, Point(3 3)@2000-01-05]}'));
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
SELECT ST_AsText(geoMeasure(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', '{[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03],[5.5@2000-01-04, 5.5@2000-01-05]}'));

SELECT ST_AsText(geoMeasure(tgeompoint 'Point(1 1 1)@2000-01-01', '5@2000-01-01'));
SELECT ST_AsText(geoMeasure(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', '{5@2000-01-01, 7@2000-01-02, 5@2000-01-03}'));
SELECT ST_AsText(geoMeasure(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', '[5@2000-01-01, 7@2000-01-02, 5@2000-01-03]'));
SELECT ST_AsText(geoMeasure(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', '{[5@2000-01-01, 7@2000-01-02, 5@2000-01-03],[5@2000-01-04, 5@2000-01-05]}'));
SELECT ST_AsText(geoMeasure(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', '5.5@2000-01-01'));
SELECT ST_AsText(geoMeasure(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', '{5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03}'));
SELECT ST_AsText(geoMeasure(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', '[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03]'));
SELECT ST_AsText(geoMeasure(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', '{[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03],[5.5@2000-01-04, 5.5@2000-01-05]}'));

-------------------------------------------------------------------------------

