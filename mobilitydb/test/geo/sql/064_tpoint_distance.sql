-------------------------------------------------------------------------------

-- NULL
SELECT round(tgeompoint '[Point(1 1)@2001-01-02, Point(2 2)@2001-01-03)' <-> tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-03, Point(2 2)@2001-01-04}', 6);
SELECT round(tgeompoint '{[Point(1 1)@2001-01-02, Point(2 2)@2001-01-03)}' <-> tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-03, Point(2 2)@2001-01-04}', 6);
SELECT round(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02),(Point(1 1)@2001-01-03, Point(2 2)@2001-01-04]}' <-> tgeompoint '(Point(1 1)@2001-01-02, Point(2 2)@2001-01-03)', 6);
SELECT round(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02),(Point(1 1)@2001-01-03, Point(2 2)@2001-01-04]}' <-> tgeompoint '{(Point(1 1)@2001-01-02, Point(2 2)@2001-01-03)}', 6);
/* Distance to a non-point geometry is computed natively for a temporal
 * geometry point with linear interpolation */
SELECT round(geometry 'Linestring(1 1,2 2)' <-> tgeompoint '[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]', 6);
SELECT round(tgeompoint '[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]' <-> geometry 'Linestring(1 1,2 2)', 6);
/* The distance turns where the point passes nearest, off its segment too */
SELECT round(tgeompoint '[Point(0 0)@2001-01-01, Point(2 0)@2001-01-03]' <-> geometry 'Point(1 1)', 6);
SELECT round(geometry 'Point(1 1)' <-> tgeompoint '[Point(0 0)@2001-01-01, Point(3 0)@2001-01-03]', 6);
SELECT round(tgeompoint '[Point(0 0 0)@2001-01-01, Point(2 0 0)@2001-01-03]' <-> geometry 'Point(1 1 1)', 6);
SELECT round(tgeogpoint '[Point(0 0)@2001-01-01, Point(2 0)@2001-01-03]' <-> geography 'Point(1 1)', 3);
SELECT round(minValue(tgeompoint '[Point(0 0)@2001-01-01, Point(3 0)@2001-01-03]' <-> geometry 'Point(1 1)'), 6);
SELECT round(nearestApproachDistance(tgeompoint '[Point(0 0)@2001-01-01, Point(3 0)@2001-01-03]', geometry 'Point(1 1)'), 6);
/* A nearest point just inside the bound makes an instant, unless it is dated on the bound */
SELECT numInstants(tgeompoint '[Point(0 0)@2001-01-01, Point(1 0)@2001-01-02]' <-> geometry 'Point(1e-7 1)');
SELECT numInstants(tgeompoint '[Point(0 0)@2001-01-01, Point(1 0)@2001-01-02]' <-> geometry 'Point(1e-12 1)');
/* A nearest point at a bound of the segment makes no instant */
SELECT round(tgeompoint '[Point(0 0)@2001-01-01, Point(2 0)@2001-01-03]' <-> geometry 'Point(5 1)', 6);
/* A temporal point of any other interpolation has a value at its instants
 * alone, the distance of each to the geometry */
SELECT round(tgeompoint 'Point(25 40)@2001-01-01' <-> geometry 'Linestring(10 10,50 10,50 50)', 6);
SELECT round(geometry 'Linestring(10 10,50 10,50 50)' <-> tgeompoint '{Point(25 40)@2001-01-01, Point(26 40)@2001-01-02}', 6);
SELECT round(tgeompoint 'Interp=Step;[Point(25 40)@2001-01-01, Point(26 40)@2001-01-02]' <-> geometry 'Polygon((10 10,50 10,50 50,10 10))', 6);
SELECT round(tgeompoint 'Interp=Step;{[Point(25 40)@2001-01-01, Point(26 40)@2001-01-02],[Point(40 20)@2001-01-03]}' <-> geometry 'Polygon((10 10,50 10,50 50,10 10))', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02}' <-> geometry 'Linestring(1 1 0,3 3 0)', 6);
SELECT round(tgeogpoint '{Point(4.35 50.85)@2001-01-01, Point(4.40 50.85)@2001-01-02}' <-> geography 'Linestring(4.35 50.80,4.40 50.80)', 6);
/* Errors */
SELECT round(geometry 'srid=5676;Point(2 2)' <-> tgeompoint '[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]', 6);
SELECT round(geometry 'Point(2 2 2)' <-> tgeompoint '[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]', 6);
SELECT round(tgeompoint '[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]' <-> geometry 'srid=5676;Point(2 2)', 6);
SELECT round(tgeompoint '[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]' <-> geometry 'Point(2 2 2)', 6);
SELECT round(tgeompoint 'SRID=5676;[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]' <-> tgeompoint '[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2001-01-01, Point(3 3 3)@2001-01-03]' <-> tgeompoint '[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]', 6);


SELECT round(geometry 'Point(1 1)' <-> tgeompoint 'Point(2 2)@2001-01-01', 6);
SELECT round(geometry 'Point(1 1)' <-> tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}', 6);
SELECT round(geometry 'Point(1 1)' <-> tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]', 6);
SELECT round(geometry 'Point(2 2)' <-> tgeompoint '[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]', 6);
SELECT round(geometry 'Point(1 1)' <-> tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', 6);

SELECT round(geometry 'Point empty' <-> tgeompoint 'Point(2 2)@2001-01-01', 6);
SELECT round(geometry 'Point empty' <-> tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}', 6);
SELECT round(geometry 'Point empty' <-> tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]', 6);
SELECT round(geometry 'Point empty' <-> tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', 6);

SELECT round(geometry 'Point(1 1 1)' <-> tgeompoint 'Point(2 2 2)@2001-01-01', 6);
SELECT round(geometry 'Point(1 1 1)' <-> tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}', 6);
SELECT round(geometry 'Point(1 1 1)' <-> tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]', 6);
SELECT round(geometry 'Point(1 1 1)' <-> tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', 6);

SELECT round(geometry 'Point Z empty' <-> tgeompoint 'Point(2 2 2)@2001-01-01', 6);
SELECT round(geometry 'Point Z empty' <-> tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}', 6);
SELECT round(geometry 'Point Z empty' <-> tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]', 6);
SELECT round(geometry 'Point Z empty' <-> tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', 6);

SELECT round(geography 'Point(-90 0)' <-> tgeogpoint 'Point(0 -90)@2001-01-01', 6);
SELECT round(geography 'Point(-90 0)' <-> tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}', 6);
SELECT round(geography 'Point(-90 0)' <-> tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]', 6);
SELECT round(geography 'Point(-90 0)' <-> tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', 6);

SELECT round(geography 'Point empty' <-> tgeogpoint 'Point(0 -90)@2001-01-01', 6);
SELECT round(geography 'Point empty' <-> tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}', 6);
SELECT round(geography 'Point empty' <-> tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]', 6);
SELECT round(geography 'Point empty' <-> tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', 6);

SELECT round(geography 'Point(-90 0 100)' <-> tgeogpoint 'Point(0 -90 100)@2001-01-01', 6);
SELECT round(geography 'Point(-90 0 100)' <-> tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}', 6);
SELECT round(geography 'Point(-90 0 100)' <-> tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]', 6);
SELECT round(geography 'Point(-90 0 100)' <-> tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', 6);

SELECT round(geography 'Point Z empty' <-> tgeogpoint 'Point(0 -90 100)@2001-01-01', 6);
SELECT round(geography 'Point Z empty' <-> tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}', 6);
SELECT round(geography 'Point Z empty' <-> tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]', 6);
SELECT round(geography 'Point Z empty' <-> tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', 6);

SELECT round(tgeompoint 'Point(1 1)@2001-01-01' <-> geometry 'Point(1 1)', 6);
SELECT round(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <-> geometry 'Point(1 1)', 6);
SELECT round(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <-> geometry 'Point(1 1)', 6);
SELECT round(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <-> geometry 'Point(1 1)', 6);

SELECT round(tgeompoint 'Point(1 1)@2001-01-01' <-> geometry 'Point empty', 6);
SELECT round(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <-> geometry 'Point empty', 6);
SELECT round(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <-> geometry 'Point empty', 6);
SELECT round(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <-> geometry 'Point empty', 6);

SELECT round(tgeompoint 'Point(1 1 1)@2001-01-01' <-> geometry 'Point(1 1 1)', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <-> geometry 'Point(1 1 1)', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <-> geometry 'Point(1 1 1)', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <-> geometry 'Point(1 1 1)', 6);

SELECT round(tgeompoint 'Point(1 1 1)@2001-01-01' <-> geometry 'Point Z empty', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <-> geometry 'Point Z empty', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <-> geometry 'Point Z empty', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <-> geometry 'Point Z empty', 6);

SELECT round(tgeogpoint 'Point(-90 0)@2001-01-01' <-> geography 'Point(-90 0)', 6);
SELECT round(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}' <-> geography 'Point(-90 0)', 6);
SELECT round(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]' <-> geography 'Point(-90 0)', 6);
SELECT round(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}' <-> geography 'Point(-90 0)', 6);

SELECT round(tgeogpoint 'Point(-90 0)@2001-01-01' <-> geography 'Point empty', 6);
SELECT round(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}' <-> geography 'Point empty', 6);
SELECT round(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]' <-> geography 'Point empty', 6);
SELECT round(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}' <-> geography 'Point empty', 6);

SELECT round(tgeogpoint 'Point(-90 0 100)@2001-01-01' <-> geography 'Point(-90 0 100)', 6);
SELECT round(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}' <-> geography 'Point(-90 0 100)', 6);
SELECT round(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]' <-> geography 'Point(-90 0 100)', 6);
SELECT round(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}' <-> geography 'Point(-90 0 100)', 6);

SELECT round(tgeogpoint 'Point(-90 0 100)@2001-01-01' <-> geography 'Point Z empty', 6);
SELECT round(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}' <-> geography 'Point Z empty', 6);
SELECT round(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]' <-> geography 'Point Z empty', 6);
SELECT round(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}' <-> geography 'Point Z empty', 6);

SELECT round(tgeompoint 'Point(1 1)@2001-01-01' <-> tgeompoint 'Point(2 2)@2001-01-01', 6);
SELECT round(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <-> tgeompoint 'Point(2 2)@2001-01-01', 6);
SELECT round(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <-> tgeompoint 'Point(2 2)@2001-01-01', 6);
SELECT round(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <-> tgeompoint 'Point(2 2)@2001-01-01', 6);
SELECT round(tgeompoint 'Point(1 1)@2001-01-01' <-> tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}', 6);
SELECT round(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <-> tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}', 6);
SELECT round(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <-> tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}', 6);
SELECT round(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <-> tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}', 6);
SELECT round(tgeompoint 'Point(1 1)@2001-01-01' <-> tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]', 6);
SELECT round(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <-> tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]', 6);
SELECT round(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <-> tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]', 6);
SELECT round(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <-> tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]', 6);
SELECT round(tgeompoint 'Point(1 1)@2001-01-01' <-> tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', 6);
SELECT round(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <-> tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', 6);
SELECT round(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <-> tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', 6);
SELECT round(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <-> tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', 6);

SELECT round(tgeompoint 'Point(1 1 1)@2001-01-01' <-> tgeompoint 'Point(2 2 2)@2001-01-01', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <-> tgeompoint 'Point(2 2 2)@2001-01-01', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <-> tgeompoint 'Point(2 2 2)@2001-01-01', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <-> tgeompoint 'Point(2 2 2)@2001-01-01', 6);
SELECT round(tgeompoint 'Point(1 1 1)@2001-01-01' <-> tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <-> tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <-> tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <-> tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}', 6);
SELECT round(tgeompoint 'Point(1 1 1)@2001-01-01' <-> tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <-> tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <-> tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <-> tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]', 6);
SELECT round(tgeompoint 'Point(1 1 1)@2001-01-01' <-> tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <-> tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <-> tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <-> tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', 6);

SELECT round(tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]' <-> tgeompoint 'Interp=Step;[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02]', 6);

-- A linear and a step operand: the distance is least where the linear point
-- passes the step point, as it is against a linear operand
SELECT round(tgeompoint '[Point(0 0)@2001-01-01, Point(2 0)@2001-01-03]' <-> tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(1 1)@2001-01-03]', 6);
SELECT round(tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(1 1)@2001-01-03]' <-> tgeompoint '[Point(0 0)@2001-01-01, Point(2 0)@2001-01-03]', 6);
SELECT round(tgeompoint '[Point(0 0)@2001-01-01, Point(2 0)@2001-01-03]' <-> tgeompoint '[Point(1 1)@2001-01-01, Point(1 1)@2001-01-03]', 6);

SELECT round(tgeogpoint 'Point(-90 0)@2001-01-01' <-> tgeogpoint 'Point(0 -90)@2001-01-01', 6);
SELECT round(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}' <-> tgeogpoint 'Point(0 -90)@2001-01-01', 6);
SELECT round(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]' <-> tgeogpoint 'Point(0 -90)@2001-01-01', 6);
SELECT round(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}' <-> tgeogpoint 'Point(0 -90)@2001-01-01', 6);
SELECT round(tgeogpoint 'Point(-90 0)@2001-01-01' <-> tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}', 6);
SELECT round(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}' <-> tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}', 6);
SELECT round(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]' <-> tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}', 6);
SELECT round(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}' <-> tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}', 6);
SELECT round(tgeogpoint 'Point(-90 0)@2001-01-01' <-> tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]', 6);
SELECT round(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}' <-> tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]', 6);
SELECT round(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]' <-> tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]', 6);
SELECT round(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}' <-> tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]', 6);
SELECT round(tgeogpoint 'Point(-90 0)@2001-01-01' <-> tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', 6);
SELECT round(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}' <-> tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', 6);
SELECT round(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]' <-> tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', 6);
SELECT round(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}' <-> tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', 6);

SELECT round(tgeogpoint 'Point(-90 0 100)@2001-01-01' <-> tgeogpoint 'Point(0 -90 100)@2001-01-01', 6);
SELECT round(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}' <-> tgeogpoint 'Point(0 -90 100)@2001-01-01', 6);
SELECT round(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]' <-> tgeogpoint 'Point(0 -90 100)@2001-01-01', 6);
SELECT round(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}' <-> tgeogpoint 'Point(0 -90 100)@2001-01-01', 6);
SELECT round(tgeogpoint 'Point(-90 0 100)@2001-01-01' <-> tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}', 6);
SELECT round(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}' <-> tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}', 6);
SELECT round(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]' <-> tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}', 6);
SELECT round(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}' <-> tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}', 6);
SELECT round(tgeogpoint 'Point(-90 0 100)@2001-01-01' <-> tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]', 6);
SELECT round(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}' <-> tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]', 6);
SELECT round(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]' <-> tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]', 6);
SELECT round(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}' <-> tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]', 6);
SELECT round(tgeogpoint 'Point(-90 0 100)@2001-01-01' <-> tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', 6);
SELECT round(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}' <-> tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', 6);
SELECT round(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]' <-> tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', 6);
SELECT round(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}' <-> tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', 6);

-------------------------------------------------------------------------------

SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2001-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint 'Interp=Step;{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2001-01-01', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint 'Interp=Step;{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', geometry 'Linestring empty'));

SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2001-01-01, Point(1 1)@2001-01-02]', geometry 'Linestring(1 1,3 3)'));

SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0)@2001-01-01', geography 'Linestring(90 0,0 60)'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', geography 'Linestring(90 0,0 60)'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', geography 'Linestring(90 0,0 60)'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', geography 'Linestring(90 0,0 60)'),6));
SELECT asText(NearestApproachInstant(tgeogpoint 'Interp=Step;[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', geography 'Linestring(90 0,0 60)'));
SELECT asText(NearestApproachInstant(tgeogpoint 'Interp=Step;{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', geography 'Linestring(90 0,0 60)'));
SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0)@2001-01-01', geography 'Linestring empty'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', geography 'Linestring empty'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', geography 'Linestring empty'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', geography 'Linestring empty'),6));
SELECT asText(NearestApproachInstant(tgeogpoint 'Interp=Step;[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', geography 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeogpoint 'Interp=Step;{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', geography 'Linestring empty'));
-- 1 degree north and 0.998 degree east of the end of the line: the spheroid ranks north nearer, the sphere east
SELECT asText(NearestApproachInstant(tgeogpoint '{Point(0 1)@2001-01-01, Point(0.998 0)@2001-01-02}', geography 'Linestring(0 0,0 -0.5)'));
SELECT asText(NearestApproachInstant(tgeogpoint '{Point(0 1)@2001-01-01, Point(0.998 0)@2001-01-02}', geography 'Linestring(0 0,0 -0.5)', false));
SELECT asText(NearestApproachInstant(tgeogpoint '{[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02],[Point(0.998 0)@2001-01-03, Point(2 0)@2001-01-04]}', geography 'Linestring(0 0,0 -0.5)'));
SELECT asText(NearestApproachInstant(tgeogpoint '{[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02],[Point(0.998 0)@2001-01-03, Point(2 0)@2001-01-04]}', geography 'Linestring(0 0,0 -0.5)', false));
SELECT asText(NearestApproachInstant(tgeogpoint '{Point(0 1)@2001-01-01, Point(0.998 0)@2001-01-02}', geography 'Polygon((0 0,0 -0.5,-0.5 -0.5,-0.5 0,0 0))'));
SELECT asText(NearestApproachInstant(tgeogpoint '{Point(0 1)@2001-01-01, Point(0.998 0)@2001-01-02}', geography 'Polygon((0 0,0 -0.5,-0.5 -0.5,-0.5 0,0 0))', false));
SELECT round(NearestApproachDistance(tgeogpoint '{Point(0 1)@2001-01-01, Point(0.998 0)@2001-01-02}', geography 'Linestring(0 0,0 -0.5)'), 3);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(0 1)@2001-01-01, Point(0.998 0)@2001-01-02}', geography 'Linestring(0 0,0 -0.5)', false), 3);

SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeompoint 'Point(1 1)@2001-01-01'));
SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'));
SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'));
SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'));

SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeompoint 'Point(1 1)@2001-01-01'));
SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'));
SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'));
SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'));

SELECT asText(NearestApproachInstant(geography 'Linestring(90 0,0 60)', tgeogpoint 'Point(-90 0)@2001-01-01'));
SELECT asText(NearestApproachInstant(geography 'Linestring(90 0,0 60)', tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}'));
SELECT asText(NearestApproachInstant(geography 'Linestring(90 0,0 60)', tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]'));
SELECT asText(NearestApproachInstant(geography 'Linestring(90 0,0 60)', tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'));

SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeogpoint 'Point(-90 0)@2001-01-01'));
SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}'));
SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]'));
SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'));

SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint 'Point(2 2)@2001-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint 'Point(2 2)@2001-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint 'Point(2 2)@2001-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint 'Point(2 2)@2001-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'));

SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02)', tgeompoint '[Point(3 3)@2001-01-01, Point(2 2)@2001-01-02)'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02], (Point(1 1)@2001-01-03, Point(0 0)@2001-01-04]}', tgeompoint '[Point(3 3)@2001-01-01, Point(3 3)@2001-01-02, Point(2 2)@2001-01-03, Point(3 3)@2001-01-04)'));

SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint 'Point(2 2 2)@2001-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint 'Point(2 2 2)@2001-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint 'Point(2 2 2)@2001-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint 'Point(2 2 2)@2001-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'));

SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0)@2001-01-01', tgeogpoint 'Point(0 -90)@2001-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', tgeogpoint 'Point(0 -90)@2001-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', tgeogpoint 'Point(0 -90)@2001-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', tgeogpoint 'Point(0 -90)@2001-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0)@2001-01-01', tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0)@2001-01-01', tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-03]', tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0)@2001-01-01', tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-03]', tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'),6));

SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0 100)@2001-01-01', tgeogpoint 'Point(0 -90 100)@2001-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}', tgeogpoint 'Point(0 -90 100)@2001-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]', tgeogpoint 'Point(0 -90 100)@2001-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', tgeogpoint 'Point(0 -90 100)@2001-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0 100)@2001-01-01', tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}', tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]', tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0 100)@2001-01-01', tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}', tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-03]', tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0 100)@2001-01-01', tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}', tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-03]', tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'),6));

/* Errors */
SELECT NearestApproachInstant(tgeompoint 'Point(1 1)@2001-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT NearestApproachInstant(tgeompoint 'Point(1 1)@2001-01-01', geometry 'Linestring(1 1 1,2 2 2)');
SELECT NearestApproachInstant(geometry 'SRID=5676;Linestring(1 1,2 2)', tgeompoint 'Point(1 1)@2001-01-01');
SELECT NearestApproachInstant(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint 'Point(1 1)@2001-01-01');
SELECT NearestApproachInstant(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint 'SRID=5676;Point(1 1)@2001-01-01');
SELECT NearestApproachInstant(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint 'Point(1 1 1)@2001-01-01');

--------------------------------------------------------

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2001-01-01', geometry 'Linestring(0 0,3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', geometry 'Linestring(0 0,3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', geometry 'Linestring(0 0,3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', geometry 'Linestring(0 0,3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', geometry 'Linestring(0 0,3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', geometry 'Linestring(0 0,3 3)'), 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2001-01-01', geometry 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', geometry 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', geometry 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', geometry 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', geometry 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', geometry 'Linestring empty'), 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2001-01-01', geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'), 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2001-01-01', geometry 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', geometry 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', geometry 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', geometry 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', geometry 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', geometry 'Linestring Z empty'), 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0)@2001-01-01', geography 'Linestring(90 0,0 60)'), 0);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', geography 'Linestring(90 0,0 60)'), 0);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', geography 'Linestring(90 0,0 60)'), 0);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', geography 'Linestring(90 0,0 60)'), 0);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0)@2001-01-01', geography 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', geography 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', geography 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', geography 'Linestring empty'), 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0 100)@2001-01-01', geography 'Linestring(90 0 0,0 90 100)'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}', geography 'Linestring(90 0 0,0 90 100)'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]', geography 'Linestring(90 0 0,0 90 100)'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', geography 'Linestring(90 0 0,0 90 100)'), 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0 100)@2001-01-01', geography 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}', geography 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]', geography 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', geography 'Linestring Z empty'), 6);

SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeompoint 'Point(1 1)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);

SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeompoint 'Point(1 1)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);

SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint 'Point(1 1 1)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);

SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeompoint 'Point(1 1 1)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);

SELECT round(NearestApproachDistance(geography 'Linestring(90 0,0 60)', tgeogpoint 'Point(-90 0)@2001-01-01'), 0);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0,0 60)', tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}'), 0);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0,0 60)', tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]'), 0);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0,0 60)', tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'), 0);

SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeogpoint 'Point(-90 0)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'), 6);

SELECT round(NearestApproachDistance(geography 'Linestring(90 0 0,0 90 100)', tgeogpoint 'Point(-90 0 100)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0 0,0 90 100)', tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0 0,0 90 100)', tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0 0,0 90 100)', tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'), 6);

SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeogpoint 'Point(-90 0 100)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'), 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint 'Point(2 2)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint 'Point(2 2)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint 'Point(2 2)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint 'Point(2 2)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint 'Point(2 2 2)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint 'Point(2 2 2)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint 'Point(2 2 2)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint 'Point(2 2 2)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0)@2001-01-01', tgeogpoint 'Point(0 -90)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', tgeogpoint 'Point(0 -90)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', tgeogpoint 'Point(0 -90)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', tgeogpoint 'Point(0 -90)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0)@2001-01-01', tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0)@2001-01-01', tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0)@2001-01-01', tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'), 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0 100)@2001-01-01', tgeogpoint 'Point(0 -90 100)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}', tgeogpoint 'Point(0 -90 100)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]', tgeogpoint 'Point(0 -90 100)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', tgeogpoint 'Point(0 -90 100)@2001-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0 100)@2001-01-01', tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}', tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]', tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0 100)@2001-01-01', tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}', tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]', tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0 100)@2001-01-01', tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}', tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]', tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'), 6);

SELECT round((stbox 'STBOX XT(((0,0),(1,1)),[2001-01-01,2001-01-02])' |=| stbox 'STBOX XT(((2,2),(2,3)),[2001-01-01,2001-01-02])'), 6);
SELECT round((stbox 'STBOX XT(((0,0),(1,1)),[2001-01-01,2001-01-02])' |=| stbox 'STBOX XT(((2,2),(3,3)),[2001-01-03,2001-01-04])'), 6);
SELECT round((stbox 'GEODSTBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' |=| stbox 'GEODSTBOX ZT(((2,2,2),(3,3,3)),[2001-01-01,2001-01-02])'), 6);
-- 3D
SELECT round((stbox 'STBOX ZT(((0,0,0),(1,1,1)),[2001-01-01,2001-01-02])' |=| stbox 'STBOX ZT(((2,2,2),(3,3,3)),[2001-01-01,2001-01-02])'), 6);

SELECT round((stbox 'STBOX XT(((1,1),(1,1)),[2001-01-01,2001-01-02])' |=| geometry 'Point empty'), 6);
SELECT round((stbox 'STBOX XT(((1,1),(1,1)),[2001-01-01,2001-01-02])' |=| geometry 'Point(0 0)'), 6);
SELECT round((stbox 'STBOX ZT(((1,1,1),(1,1,1)),[2001-01-01,2001-01-02])' |=| geometry 'Point (0 0 0)'), 6);

SELECT round((geometry 'Point empty' |=| stbox 'STBOX XT(((1,1),(1,1)),[2001-01-01,2001-01-02])'), 6);
SELECT round((geometry 'Point(0 0)' |=| stbox 'STBOX XT(((1,1),(1,1)),[2001-01-01,2001-01-02])'), 6);
SELECT round((geometry 'Point (0 0 0)' |=| stbox 'STBOX ZT(((1,1,1),(1,1,1)),[2001-01-01,2001-01-02])'), 6);

/* Errors */
SELECT stbox 'STBOX XT(((1,1),(1,1)),[2001-01-01,2001-01-02])' |=| geometry 'SRID=5676;Point(0 0)';
SELECT round((stbox 'STBOX XT(((1,1),(1,1)),[2001-01-01,2001-01-02])' |=| geometry 'Point(0 0 0)'), 6);
SELECT stbox 'STBOX X((1.0,2.0),(3.0,4.0))' |=| stbox 'STBOX Z((1.0,2.0,3.0),(4.0,5.0,6.0))';
SELECT round((stbox 'GEODSTBOX ZT(((1.0,2.0,3.0),(1.0,2.0,3.0)),[2001-01-03,2001-01-04])' |=| geography 'Point(0 0)'), 3);

SELECT round((tgeompoint 'Point(1 1)@2001-01-01' |=| stbox 'STBOX XT(((2,2),(2,2)),[2001-01-01,2001-01-02])'), 6);
SELECT round((tgeompoint 'Point(1 1)@2001-01-01' |=| stbox 'STBOX XT(((2,2),(2,2)),[2001-01-02,2001-01-03])'), 6);

-- A box with no period measures the whole temporal point
SELECT round((tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]' |=| stbox 'STBOX X((5,5),(6,6))'), 6);
-- A box with a period measures the part of the temporal point inside it
SELECT round((tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]' |=| stbox 'STBOX XT(((5,5),(6,6)),[2001-01-01, 2001-01-01 12:00])'), 6);
-- No part of the temporal point is inside the period
SELECT round((tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]' |=| stbox 'STBOX XT(((5,5),(6,6)),[2001-02-01, 2001-02-02])'), 6);
-- The two periods meet while no value of a discrete temporal point is inside
SELECT round((tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-03}' |=| stbox 'STBOX XT(((5,5),(6,6)),[2001-01-02, 2001-01-02 12:00])'), 6);

/* Errors */
SELECT round((tgeompoint 'Point(1 1 1)@2001-01-01' |=| stbox 'STBOX XT(((2,2),(2,2)),[2001-01-01,2001-01-02])'), 6);
SELECT round((tgeogpoint 'Point(1 1)@2001-01-01' |=| stbox 'GEODSTBOX ZT(((2,2,2),(2,2,2)),[2001-01-01,2001-01-02])'), 3);

SELECT round((stbox 'STBOX XT(((2,2),(2,2)),[2001-01-01,2001-01-02])' |=| tgeompoint 'Point(1 1)@2001-01-01' ), 6);
SELECT round((stbox 'STBOX XT(((2,2),(2,2)),[2001-01-02,2001-01-03])' |=| tgeompoint 'Point(1 1)@2001-01-01' ), 6);

SELECT round((tgeompoint 'Point(1 1)@2001-01-01' |=| geometry 'Linestring(0 0,3 3)'), 6);
SELECT round((tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' |=| geometry 'Linestring(0 0,3 3)'), 6);
SELECT round((tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' |=| geometry 'Linestring(0 0,3 3)'), 6);
SELECT round((tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' |=| geometry 'Linestring(0 0,3 3)'), 6);

SELECT round((tgeompoint 'Point(1 1)@2001-01-01' |=| geometry 'Linestring empty'), 6);
SELECT round((tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' |=| geometry 'Linestring empty'), 6);
SELECT round((tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' |=| geometry 'Linestring empty'), 6);
SELECT round((tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' |=| geometry 'Linestring empty'), 6);

SELECT round((tgeompoint 'Point(1 1 1)@2001-01-01' |=| geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round((tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' |=| geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round((tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' |=| geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' |=| geometry 'Linestring(0 0 0,3 3 3)'), 6);

SELECT round((tgeompoint 'Point(1 1 1)@2001-01-01' |=| geometry 'Linestring Z empty'), 6);
SELECT round((tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' |=| geometry 'Linestring Z empty'), 6);
SELECT round((tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' |=| geometry 'Linestring Z empty'), 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' |=| geometry 'Linestring Z empty'), 6);

SELECT round((tgeogpoint 'Point(-90 0)@2001-01-01' |=| geography 'Linestring(90 0,0 60)'), 0);
SELECT round((tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}' |=| geography 'Linestring(90 0,0 60)'), 0);
SELECT round((tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]' |=| geography 'Linestring(90 0,0 60)'), 0);
SELECT round((tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}' |=| geography 'Linestring(90 0,0 60)'), 0);

SELECT round((tgeogpoint 'Point(-90 0)@2001-01-01' |=| geography 'Linestring empty'), 6);
SELECT round((tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}' |=| geography 'Linestring empty'), 6);
SELECT round((tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]' |=| geography 'Linestring empty'), 6);
SELECT round((tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}' |=| geography 'Linestring empty'), 6);

SELECT round((tgeogpoint 'Point(-90 0 100)@2001-01-01' |=| geography 'Linestring(90 0 0,0 90 100)'), 6);
SELECT round((tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}' |=| geography 'Linestring(90 0 0,0 90 100)'), 6);
SELECT round((tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]' |=| geography 'Linestring(90 0 0,0 90 100)'), 6);
SELECT round((tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}' |=| geography 'Linestring(90 0 0,0 90 100)'), 6);

SELECT round((tgeogpoint 'Point(-90 0 100)@2001-01-01' |=| geography 'Linestring Z empty'), 6);
SELECT round((tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}' |=| geography 'Linestring Z empty'), 6);
SELECT round((tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]' |=| geography 'Linestring Z empty'), 6);
SELECT round((tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}' |=| geography 'Linestring Z empty'), 6);

SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeompoint 'Point(1 1)@2001-01-01'), 6);
SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'), 6);
SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'), 6);
SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);

SELECT round((geometry 'Linestring empty' |=| tgeompoint 'Point(1 1)@2001-01-01'), 6);
SELECT round((geometry 'Linestring empty' |=| tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'), 6);
SELECT round((geometry 'Linestring empty' |=| tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'), 6);
SELECT round((geometry 'Linestring empty' |=| tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);

SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeompoint 'Point(1 1 1)@2001-01-01'), 6);
SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}'), 6);
SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]'), 6);
SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);

SELECT round((geometry 'Linestring Z empty' |=| tgeompoint 'Point(1 1 1)@2001-01-01'), 6);
SELECT round((geometry 'Linestring Z empty' |=| tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}'), 6);
SELECT round((geometry 'Linestring Z empty' |=| tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]'), 6);
SELECT round((geometry 'Linestring Z empty' |=| tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);

SELECT round((geography 'Linestring(90 0,0 60)' |=| tgeogpoint 'Point(-90 0)@2001-01-01'), 0);
SELECT round((geography 'Linestring(90 0,0 60)' |=| tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}'), 0);
SELECT round((geography 'Linestring(90 0,0 60)' |=| tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]'), 0);
SELECT round((geography 'Linestring(90 0,0 60)' |=| tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'), 0);

SELECT round((geography 'Linestring empty' |=| tgeogpoint 'Point(-90 0)@2001-01-01'), 6);
SELECT round((geography 'Linestring empty' |=| tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}'), 6);
SELECT round((geography 'Linestring empty' |=| tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]'), 6);
SELECT round((geography 'Linestring empty' |=| tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'), 6);

SELECT round((geography 'Linestring(90 0 0,0 90 100)' |=| tgeogpoint 'Point(-90 0 100)@2001-01-01'), 6);
SELECT round((geography 'Linestring(90 0 0,0 90 100)' |=| tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}'), 6);
SELECT round((geography 'Linestring(90 0 0,0 90 100)' |=| tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]'), 6);
SELECT round((geography 'Linestring(90 0 0,0 90 100)' |=| tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'), 6);

SELECT round((geography 'Linestring Z empty' |=| tgeogpoint 'Point(-90 0 100)@2001-01-01'), 6);
SELECT round((geography 'Linestring Z empty' |=| tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}'), 6);
SELECT round((geography 'Linestring Z empty' |=| tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]'), 6);
SELECT round((geography 'Linestring Z empty' |=| tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'), 6);

SELECT round((tgeompoint 'Point(1 1)@2001-01-01' |=| tgeompoint 'Point(2 2)@2001-01-01'), 6);
SELECT round((tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' |=| tgeompoint 'Point(2 2)@2001-01-01'), 6);
SELECT round((tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' |=| tgeompoint 'Point(2 2)@2001-01-01'), 6);
SELECT round((tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' |=| tgeompoint 'Point(2 2)@2001-01-01'), 6);
SELECT round((tgeompoint 'Point(1 1)@2001-01-01' |=| tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'), 6);
SELECT round((tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' |=| tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'), 6);
SELECT round((tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' |=| tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'), 6);
SELECT round((tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' |=| tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'), 6);
SELECT round((tgeompoint 'Point(1 1)@2001-01-01' |=| tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'), 6);
SELECT round((tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' |=| tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'), 6);
SELECT round((tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' |=| tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'), 6);
SELECT round((tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' |=| tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'), 6);
SELECT round((tgeompoint 'Point(1 1)@2001-01-01' |=| tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);
SELECT round((tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' |=| tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);
SELECT round((tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' |=| tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);
SELECT round((tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' |=| tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);

SELECT round((tgeompoint 'Point(1 1 1)@2001-01-01' |=| tgeompoint 'Point(2 2 2)@2001-01-01'), 6);
SELECT round((tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' |=| tgeompoint 'Point(2 2 2)@2001-01-01'), 6);
SELECT round((tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' |=| tgeompoint 'Point(2 2 2)@2001-01-01'), 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' |=| tgeompoint 'Point(2 2 2)@2001-01-01'), 6);
SELECT round((tgeompoint 'Point(1 1 1)@2001-01-01' |=| tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'), 6);
SELECT round((tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' |=| tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'), 6);
SELECT round((tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' |=| tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'), 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' |=| tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'), 6);
SELECT round((tgeompoint 'Point(1 1 1)@2001-01-01' |=| tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'), 6);
SELECT round((tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' |=| tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'), 6);
SELECT round((tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' |=| tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'), 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' |=| tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'), 6);
SELECT round((tgeompoint 'Point(1 1 1)@2001-01-01' |=| tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);
SELECT round((tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' |=| tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);
SELECT round((tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' |=| tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' |=| tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);

SELECT round((tgeogpoint 'Point(-90 0)@2001-01-01' |=| tgeogpoint 'Point(0 -90)@2001-01-01'), 6);
SELECT round((tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}' |=| tgeogpoint 'Point(0 -90)@2001-01-01'), 6);
SELECT round((tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]' |=| tgeogpoint 'Point(0 -90)@2001-01-01'), 6);
SELECT round((tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}' |=| tgeogpoint 'Point(0 -90)@2001-01-01'), 6);
SELECT round((tgeogpoint 'Point(-90 0)@2001-01-01' |=| tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}'), 6);
SELECT round((tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}' |=| tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}'), 6);
SELECT round((tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]' |=| tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}'), 6);
SELECT round((tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}' |=| tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}'), 6);
SELECT round((tgeogpoint 'Point(-90 0)@2001-01-01' |=| tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]'), 6);
SELECT round((tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}' |=| tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]'), 6);
SELECT round((tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]' |=| tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]'), 6);
SELECT round((tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}' |=| tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]'), 6);
SELECT round((tgeogpoint 'Point(-90 0)@2001-01-01' |=| tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'), 6);
SELECT round((tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}' |=| tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'), 6);
SELECT round((tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]' |=| tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'), 6);
SELECT round((tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}' |=| tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'), 6);

SELECT round((tgeogpoint 'Point(-90 0 100)@2001-01-01' |=| tgeogpoint 'Point(0 -90 100)@2001-01-01'), 6);
SELECT round((tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}' |=| tgeogpoint 'Point(0 -90 100)@2001-01-01'), 6);
SELECT round((tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]' |=| tgeogpoint 'Point(0 -90 100)@2001-01-01'), 6);
SELECT round((tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}' |=| tgeogpoint 'Point(0 -90 100)@2001-01-01'), 6);
SELECT round((tgeogpoint 'Point(-90 0 100)@2001-01-01' |=| tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}'), 6);
SELECT round((tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}' |=| tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}'), 6);
SELECT round((tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]' |=| tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}'), 6);
SELECT round((tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}' |=| tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}'), 6);
SELECT round((tgeogpoint 'Point(-90 0 100)@2001-01-01' |=| tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]'), 6);
SELECT round((tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}' |=| tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]'), 6);
SELECT round((tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]' |=| tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]'), 6);
SELECT round((tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}' |=| tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]'), 6);
SELECT round((tgeogpoint 'Point(-90 0 100)@2001-01-01' |=| tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'), 6);
SELECT round((tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}' |=| tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'), 6);
SELECT round((tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]' |=| tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'), 6);
SELECT round((tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}' |=| tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}'), 6);

/* Errors */
SELECT NearestApproachDistance(tgeompoint 'Point(1 1)@2001-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT NearestApproachDistance(tgeompoint 'Point(1 1)@2001-01-01', geometry 'Linestring(1 1 1,2 2 2)');
SELECT NearestApproachDistance(geometry 'SRID=5676;Linestring(1 1,2 2)', tgeompoint 'Point(1 1)@2001-01-01');
SELECT NearestApproachDistance(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint 'Point(1 1)@2001-01-01');
SELECT NearestApproachDistance(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint 'SRID=5676;Point(1 1)@2001-01-01');
SELECT NearestApproachDistance(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint 'Point(1 1 1)@2001-01-01');

--------------------------------------------------------

SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1)@2001-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', geometry 'Linestring(0 0,3 3)'));

SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1)@2001-01-01', geometry 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', geometry 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', geometry 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', geometry 'Linestring empty'));

SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1 1)@2001-01-01', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'));

SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1 1)@2001-01-01', geometry 'Linestring Z empty'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', geometry 'Linestring Z empty'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', geometry 'Linestring Z empty'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', geometry 'Linestring Z empty'));

SELECT ST_AsTexT(shortestLine(tgeogpoint 'Point(-90 0)@2001-01-01', geography 'Linestring(90 0,0 60)'), 1);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', geography 'Linestring(90 0,0 60)')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', geography 'Linestring(90 0,0 60)')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', geography 'Linestring(90 0,0 60)')), 6);

SELECT ST_AsTexT(shortestLine(tgeogpoint 'Point(-90 0)@2001-01-01', geography 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', geography 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', geography 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', geography 'Linestring empty'));

SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0,3 3)', tgeompoint 'Point(1 1)@2001-01-01'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0,3 3)', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0,3 3)', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0,3 3)', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'));

SELECT ST_AsTexT(shortestLine(geometry 'Linestring empty', tgeompoint 'Point(1 1)@2001-01-01'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring empty', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring empty', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring empty', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'));

SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint 'Point(1 1 1)@2001-01-01'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'));

SELECT ST_AsTexT(shortestLine(geometry 'Linestring Z empty', tgeompoint 'Point(1 1 1)@2001-01-01'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring Z empty', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring Z empty', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring Z empty', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'));

SELECT ST_AsTexT(shortestLine(geography 'Linestring(90 0,0 60)', tgeogpoint 'Point(-90 0)@2001-01-01'), 1);
SELECT ST_AsTexT(round(shortestLine(geography 'Linestring(90 0,0 60)', tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(geography 'Linestring(90 0,0 60)', tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(geography 'Linestring(90 0,0 60)', tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}')), 6);

SELECT ST_AsTexT(shortestLine(geography 'Linestring empty', tgeogpoint 'Point(-90 0)@2001-01-01'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring empty', tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring empty', tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring empty', tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}'), 1);

SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint 'Point(2 2)@2001-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint 'Point(2 2)@2001-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint 'Point(2 2)@2001-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint 'Point(2 2)@2001-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '{Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '{[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'));

SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint 'Point(2 2 2)@2001-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint 'Point(2 2 2)@2001-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint 'Point(2 2 2)@2001-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint 'Point(2 2 2)@2001-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '{Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '{[Point(2 2 2)@2001-01-01, Point(1 1 1)@2001-01-02, Point(2 2 2)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'));

SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0)@2001-01-01', tgeogpoint 'Point(0 -90)@2001-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', tgeogpoint 'Point(0 -90)@2001-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', tgeogpoint 'Point(0 -90)@2001-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', tgeogpoint 'Point(0 -90)@2001-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0)@2001-01-01', tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', tgeogpoint '{Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0)@2001-01-01', tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', tgeogpoint '[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0)@2001-01-01', tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03}', tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03]', tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0)@2001-01-01, Point(0 0)@2001-01-02, Point(-90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}', tgeogpoint '{[Point(90 0)@2001-01-01, Point(-90 0)@2001-01-02, Point(90 0)@2001-01-03],[Point(90 90)@2001-01-04, Point(90 90)@2001-01-05]}')), 6);

SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0 100)@2001-01-01', tgeogpoint 'Point(0 -90 100)@2001-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}', tgeogpoint 'Point(0 -90 100)@2001-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]', tgeogpoint 'Point(0 -90 100)@2001-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', tgeogpoint 'Point(0 -90 100)@2001-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0 100)@2001-01-01', tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}', tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]', tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', tgeogpoint '{Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0 100)@2001-01-01', tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}', tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]', tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', tgeogpoint '[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0 100)@2001-01-01', tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03}', tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03]', tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0 100)@2001-01-01, Point(0 0 100)@2001-01-02, Point(-90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}', tgeogpoint '{[Point(90 0 100)@2001-01-01, Point(-90 0 100)@2001-01-02, Point(90 0 100)@2001-01-03],[Point(90 90 100)@2001-01-04, Point(90 90 100)@2001-01-05]}')), 6);

SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1)@2001-01-01, Point(1 1)@2001-01-02], (Point(2 2)@2001-01-04, Point(1 1)@2001-01-05]}', tgeompoint '{[Point(3 3)@2001-01-01, Point(3 3)@2001-01-02], (Point(2 2)@2001-01-04, Point(3 3)@2001-01-05]}'));
SELECT ST_AsText(shortestLine(tgeompoint 'Interp=Step;[Point(0 0)@2001-01-01, Point(2 0)@2001-01-02]','[Point(1 1)@2001-01-01,Point(1 1)@2001-01-02]'));
SELECT ST_AsText(shortestLine(tgeompoint 'Interp=Step;{(Point(0 0)@2001-01-01, Point(2 0)@2001-01-02]}','{[Point(1 1)@2001-01-01,Point(1 1)@2001-01-02]}'));
SELECT ST_AsText(shortestLine(tgeompoint '{[Point(0 0)@2001-01-01, Point(2 0)@2001-01-02]}','{(Point(1 1)@2001-01-01,Point(2 2)@2001-01-02]}'));
SELECT ST_AsText(shortestLine(tgeompoint '{[Point(0 0)@2001-01-01],(Point(0 0)@2001-01-02, Point(1 1)@2001-01-03)}','{[Point(1 3)@2001-01-01, Point(1 2)@2001-01-03]}'));
SELECT ST_AsText(shortestLine(tgeompoint '{[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02),[Point(0 0)@2001-01-03]}','{[Point(1 3)@2001-01-01, Point(1 2)@2001-01-03]}'));
SELECT ST_AsText(shortestLine(tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02)', '[Point(4 1)@2001-01-01, Point(2 1)@2001-01-02)'));
-- NULL
SELECT shortestline(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02),[Point(1 1)@2001-01-03, Point(2 2)@2001-01-04]}', tgeompoint '[Point(1 1)@2001-01-02, Point(1 1)@2001-01-03)');
SELECT shortestline(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02),[Point(1 1)@2001-01-03, Point(2 2)@2001-01-04]}', tgeompoint '{[Point(1 1)@2001-01-02, Point(1 1)@2001-01-03)}');
/* Errors */
SELECT shortestLine(tgeompoint 'Point(1 1)@2001-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT shortestLine(tgeompoint 'Point(1 1)@2001-01-01', geometry 'Linestring(1 1 1,2 2 2)');
SELECT shortestLine(geometry 'SRID=5676;Linestring(1 1,2 2)', tgeompoint 'Point(1 1)@2001-01-01');
SELECT shortestLine(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint 'Point(1 1)@2001-01-01');
SELECT shortestLine(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint 'SRID=5676;Point(1 1)@2001-01-01');
SELECT shortestLine(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint 'Point(1 1 1)@2001-01-01');
SELECT shortestLine(geography 'Linestring(90 0 0,0 90 100)', tgeogpoint 'Point(-90 0 100)@2001-01-01');
SELECT shortestLine(tgeogpoint 'Point(-90 0 100)@2001-01-01', geography 'Linestring(90 0 0,0 90 100)');

--------------------------------------------------------
-- minDistance(tgeompoint[], tgeompoint[]) — exact set-set spatial min distance
--------------------------------------------------------

-- Two singleton sets, segments crossing each other: distance is 0
SELECT minDistance(
  ARRAY[tgeompoint '[Point(0 0)@2001-01-01, Point(2 2)@2001-01-02]'],
  ARRAY[tgeompoint '[Point(0 2)@2001-01-01, Point(2 0)@2001-01-02]']);

-- Two singleton sets, parallel disjoint: exact distance is 1
SELECT round(minDistance(
  ARRAY[tgeompoint '[Point(0 0)@2001-01-01, Point(10 0)@2001-01-02]'],
  ARRAY[tgeompoint '[Point(0 1)@2001-01-01, Point(10 1)@2001-01-02]']), 6);

-- Multi-element sets: closest pair determines the answer, bbox prefilter prunes the rest
SELECT round(minDistance(
  ARRAY[tgeompoint '[Point(0 0)@2001-01-01, Point(1 0)@2001-01-02]',
        tgeompoint '[Point(100 100)@2001-01-01, Point(101 100)@2001-01-02]'],
  ARRAY[tgeompoint '[Point(0 5)@2001-01-01, Point(1 5)@2001-01-02]',
        tgeompoint '[Point(200 200)@2001-01-01, Point(201 200)@2001-01-02]']), 6);

-- Equivalence vs ST_Distance(ST_Collect, ST_Collect): same answer
WITH A AS (SELECT ARRAY[
  tgeompoint '[Point(0 0)@2001-01-01, Point(5 0)@2001-01-02]',
  tgeompoint '[Point(0 10)@2001-01-01, Point(5 10)@2001-01-02]'] AS a),
B AS (SELECT ARRAY[
  tgeompoint '[Point(0 1)@2001-01-01, Point(5 1)@2001-01-02]',
  tgeompoint '[Point(0 11)@2001-01-01, Point(5 11)@2001-01-02]'] AS b)
SELECT round(minDistance(a, b), 6) = round(ST_Distance(
  ST_Collect(ARRAY[trajectory(a[1]), trajectory(a[2])]),
  ST_Collect(ARRAY[trajectory(b[1]), trajectory(b[2])])), 6) AS bit_equivalent
FROM A, B;

-- Errors / NULL handling
SELECT minDistance(ARRAY[]::tgeompoint[], ARRAY[tgeompoint 'Point(0 0)@2001-01-01']);
SELECT minDistance(ARRAY[tgeompoint 'Point(0 0)@2001-01-01'],
                   ARRAY[tgeompoint 'SRID=4326;Point(0 0)@2001-01-01']);

--------------------------------------------------------

-- One degree of meridian near the equator measures about 110575 m on the spheroid and 111195 m on the sphere
SELECT round(tDistance(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', geography 'Point(0 0)'), 3);
SELECT round(tDistance(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', geography 'Point(0 0)', false), 3);
SELECT round(tDistance(geography 'Point(0 0)', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]'), 3);
SELECT round(tDistance(geography 'Point(0 0)', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', false), 3);
SELECT round(tDistance(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', tgeogpoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]'), 3);
SELECT round(tDistance(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', tgeogpoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]', false), 3);
SELECT round(nearestApproachDistance(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', geography 'Point(0 0)')::numeric, 3);
SELECT round(nearestApproachDistance(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', geography 'Point(0 0)', false)::numeric, 3);
SELECT round(nearestApproachDistance(geography 'Point(0 0)', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]')::numeric, 3);
SELECT round(nearestApproachDistance(geography 'Point(0 0)', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', false)::numeric, 3);
SELECT round(nearestApproachDistance(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', tgeogpoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]')::numeric, 3);
SELECT round(nearestApproachDistance(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', tgeogpoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]', false)::numeric, 3);
SELECT round(nearestApproachDistance(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', stbox 'SRID=4326;GEODSTBOX X((0,0),(0,0))')::numeric, 3);
SELECT round(nearestApproachDistance(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', stbox 'SRID=4326;GEODSTBOX X((0,0),(0,0))', false)::numeric, 3);
SELECT round(nearestApproachDistance(stbox 'SRID=4326;GEODSTBOX X((0,0),(0,0))', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]')::numeric, 3);
SELECT round(nearestApproachDistance(stbox 'SRID=4326;GEODSTBOX X((0,0),(0,0))', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', false)::numeric, 3);
SELECT asText(nearestApproachInstant(geography 'Point(0 0)', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]'));
SELECT asText(nearestApproachInstant(geography 'Point(0 0)', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', false));
SELECT asText(nearestApproachInstant(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', geography 'Point(0 0)'));
SELECT asText(nearestApproachInstant(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', geography 'Point(0 0)', false));
SELECT asText(nearestApproachInstant(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', tgeogpoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]'));
SELECT asText(nearestApproachInstant(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', tgeogpoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]', false));
SELECT ST_AsText(shortestLine(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', geography 'Point(0 0)'), 6);
SELECT ST_AsText(shortestLine(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', geography 'Point(0 0)', false), 6);
SELECT ST_AsText(shortestLine(geography 'Point(0 0)', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]'), 6);
SELECT ST_AsText(shortestLine(geography 'Point(0 0)', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', false), 6);
SELECT ST_AsText(shortestLine(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', tgeogpoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]'), 6);
SELECT ST_AsText(shortestLine(tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', tgeogpoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]', false), 6);
SELECT round(nearestApproachDistance(stbox 'SRID=4326;GEODSTBOX X((0,1),(0,2))', stbox 'SRID=4326;GEODSTBOX X((0,0),(0,0))')::numeric, 3);
SELECT round(nearestApproachDistance(stbox 'SRID=4326;GEODSTBOX X((0,1),(0,2))', stbox 'SRID=4326;GEODSTBOX X((0,0),(0,0))', false)::numeric, 3);
SELECT round(nearestApproachDistance(stbox 'SRID=4326;GEODSTBOX X((0,1),(0,2))', geography 'Point(0 0)')::numeric, 3);
SELECT round(nearestApproachDistance(stbox 'SRID=4326;GEODSTBOX X((0,1),(0,2))', geography 'Point(0 0)', false)::numeric, 3);
SELECT round(nearestApproachDistance(geography 'Point(0 0)', stbox 'SRID=4326;GEODSTBOX X((0,1),(0,2))')::numeric, 3);
SELECT round(nearestApproachDistance(geography 'Point(0 0)', stbox 'SRID=4326;GEODSTBOX X((0,1),(0,2))', false)::numeric, 3);
-- 1 degree north and 0.998 degree east of the end of the line: the spheroid ranks north nearer, the sphere east
SELECT ST_AsText(shortestLine(tgeogpoint '{Point(0 1)@2001-01-01, Point(0.998 0)@2001-01-02}', geography 'Linestring(0 0,0 -0.5)'), 6);
SELECT ST_AsText(shortestLine(tgeogpoint '{Point(0 1)@2001-01-01, Point(0.998 0)@2001-01-02}', geography 'Linestring(0 0,0 -0.5)', false), 6);
SELECT ST_AsText(shortestLine(geography 'Linestring(0 0,0 -0.5)', tgeogpoint '{Point(0 1)@2001-01-01, Point(0.998 0)@2001-01-02}'), 6);
SELECT ST_AsText(shortestLine(geography 'Linestring(0 0,0 -0.5)', tgeogpoint '{Point(0 1)@2001-01-01, Point(0.998 0)@2001-01-02}', false), 6);

-------------------------------------------------------------------------------
-- Coordinates near the largest double

-- A distance beyond the largest double raises an error
SELECT tgeompoint '[Point(-1e308 0)@2001-01-01, Point(-1e308 4)@2001-01-02]' |=| geometry 'Linestring(1e308 -1,1e308 1)';
-- The difference of two coordinates beyond the doubles, a distance within them
SELECT round(((tgeompoint '[Point(1.6e308 0.25)@2001-01-01, Point(1.6e308 0.75)@2001-01-02]' |=| geometry 'Linestring(1.7e308 0,1.7e308 1)') / 1e306)::numeric, 6);
SELECT ST_AsText(shortestLine(tgeompoint '[Point(1.6e308 0.25)@2001-01-01, Point(1.6e308 0.75)@2001-01-02]', geometry 'Linestring(1.7e308 0,1.7e308 1)'));
-- A path 2e200 from an edge of length 2
SELECT ST_AsText(shortestLine(tgeompoint '[Point(-1e200 0)@2001-01-01, Point(-1e200 4)@2001-01-02]', geometry 'Linestring(1e200 -1,1e200 1)'));
SELECT tgeompoint '[Point(-1e153 0)@2001-01-01, Point(-1e153 4)@2001-01-02]' <-> geometry 'Linestring(1e153 -1,1e153 1)';
SELECT tgeompoint '[Point(-1e200 0)@2001-01-01, Point(-1e200 4)@2001-01-02]' <-> geometry 'Linestring(1e200 -1,1e200 1)';
SELECT tgeompoint '[Point(-1e200 0)@2001-01-01, Point(-1e200 4)@2001-01-02]' <-> geometry 'Polygon((1e200 -1,2e200 -1,2e200 1,1e200 1,1e200 -1))';
-- Coordinates beyond 2^1021 and below 2^-1021 in one pair of segments in 3D
SELECT round((tgeompoint '[Point(1e308 0 1e-310)@2001-01-01, Point(-1e308 1 2e-310)@2001-01-02]' |=| geometry 'Point(0 3e-310 5)')::numeric, 6);

-------------------------------------------------------------------------------

-- Operands of different dimensions read the dimensions both have: the examples
-- of the manual, the answers in both orders, and an operation building a value
SELECT tgeompoint '[Point(0 0)@2001-01-01, Point(10 0)@2001-01-02]' |=| tgeompoint '[Point(0 0 10000)@2001-01-01, Point(10 0 10000)@2001-01-02]';
SELECT tgeompoint '[Point(0 0 10000)@2001-01-01, Point(10 0 10000)@2001-01-02]' |=| tgeompoint '[Point(0 0)@2001-01-01, Point(10 0)@2001-01-02]';
SELECT tgeompoint '[Point(0 0 0)@2001-01-01, Point(10 0 0)@2001-01-02]' |=| tgeompoint '[Point(0 0 10000)@2001-01-01, Point(10 0 10000)@2001-01-02]';
SELECT merge(tgeompoint 'Point(0 0)@2001-01-01', tgeompoint 'Point(0 0 0)@2001-01-02');
SELECT tgeompoint 'SRID=3857;Point(0 0)@2001-01-01' |=| tgeompoint 'SRID=4326;Point(0 0)@2001-01-01';
SELECT asText(NearestApproachInstant(tgeompoint '[Point(0 0 7)@2001-01-01, Point(2 0 7)@2001-01-03]', geometry 'Point(1 1)'));
SELECT asText(shortestLine(tgeompoint '[Point(0 0 7)@2001-01-01, Point(2 0 7)@2001-01-03]', geometry 'Point(1 1)'));
-- A 3D polyhedral surface is read in 2D as any 3D operand of a 2D value
SELECT tDistance(tgeompoint '[Point(0 0)@2001-01-01, Point(3 0)@2001-01-02]', geometry 'PolyhedralSurface Z(((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1)))');
SELECT round(nearestApproachDistance(tgeompoint '[Point(3 3)@2001-01-01, Point(4 4)@2001-01-02]', geometry 'PolyhedralSurface Z(((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1)))'), 6);
SELECT round(geometry 'PolyhedralSurface Z(((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1)))' |=| tgeompoint '[Point(3 3)@2001-01-01, Point(4 4)@2001-01-02]', 6);

-------------------------------------------------------------------------------
