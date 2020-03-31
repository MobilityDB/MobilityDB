-------------------------------------------------------------------------------

-- NULL
SELECT round(tgeompoint '[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03)' <-> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(2 2)@2000-01-04}', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03)}' <-> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(2 2)@2000-01-04}', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02),(Point(1 1)@2000-01-03, Point(2 2)@2000-01-04]}' <-> tgeompoint '(Point(1 1)@2000-01-02, Point(2 2)@2000-01-03)', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02),(Point(1 1)@2000-01-03, Point(2 2)@2000-01-04]}' <-> tgeompoint '{(Point(1 1)@2000-01-02, Point(2 2)@2000-01-03)}', 6);
/* Errors */
SELECT round(geometry 'Linestring(1 1,2 2)' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(geometry 'srid=5676;Point(2 2)' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(geometry 'Point(2 2 2)' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> geometry 'Linestring(1 1,2 2)', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> geometry 'srid=5676;Point(2 2)', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> geometry 'Point(2 2 2)', 6);
SELECT round(tgeompoint 'SRID=5676;[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2000-01-01, Point(3 3 3)@2000-01-03]' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);


SELECT round(geometry 'Point(1 1)' <-> tgeompoint 'Point(2 2)@2000-01-01', 6);
SELECT round(geometry 'Point(1 1)' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(geometry 'Point(1 1)' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(geometry 'Point(2 2)' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(geometry 'Point(1 1)' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);

SELECT round(geometry 'Point empty' <-> tgeompoint 'Point(2 2)@2000-01-01', 6);
SELECT round(geometry 'Point empty' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(geometry 'Point empty' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(geometry 'Point empty' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);

SELECT round(geometry 'Point(1 1 1)' <-> tgeompoint 'Point(2 2 2)@2000-01-01', 6);
SELECT round(geometry 'Point(1 1 1)' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(geometry 'Point(1 1 1)' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(geometry 'Point(1 1 1)' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);

SELECT round(geometry 'Point Z empty' <-> tgeompoint 'Point(2 2 2)@2000-01-01', 6);
SELECT round(geometry 'Point Z empty' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(geometry 'Point Z empty' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(geometry 'Point Z empty' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);

SELECT round(geography 'Point(1 1)' <-> tgeogpoint 'Point(2.5 2.5)@2000-01-01', 6);
SELECT round(geography 'Point(1 1)' <-> tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}', 6);
SELECT round(geography 'Point(1 1)' <-> tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]', 6);
SELECT round(geography 'Point(1 1)' <-> tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 6);

SELECT round(geography 'Point empty' <-> tgeogpoint 'Point(2.5 2.5)@2000-01-01', 6);
SELECT round(geography 'Point empty' <-> tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}', 6);
SELECT round(geography 'Point empty' <-> tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]', 6);
SELECT round(geography 'Point empty' <-> tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 6);

SELECT round(geography 'Point(1 1 1)' <-> tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01', 6);
SELECT round(geography 'Point(1 1 1)' <-> tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}', 6);
SELECT round(geography 'Point(1 1 1)' <-> tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]', 6);
SELECT round(geography 'Point(1 1 1)' <-> tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 6);

SELECT round(geography 'Point Z empty' <-> tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01', 6);
SELECT round(geography 'Point Z empty' <-> tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}', 6);
SELECT round(geography 'Point Z empty' <-> tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]', 6);
SELECT round(geography 'Point Z empty' <-> tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 6);

SELECT round(tgeompoint 'Point(1 1)@2000-01-01' <-> geometry 'Point(1 1)', 6);
SELECT round(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> geometry 'Point(1 1)', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> geometry 'Point(1 1)', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> geometry 'Point(1 1)', 6);

SELECT round(tgeompoint 'Point(1 1)@2000-01-01' <-> geometry 'Point empty', 6);
SELECT round(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> geometry 'Point empty', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> geometry 'Point empty', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> geometry 'Point empty', 6);

SELECT round(tgeompoint 'Point(1 1 1)@2000-01-01' <-> geometry 'Point(1 1 1)', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> geometry 'Point(1 1 1)', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> geometry 'Point(1 1 1)', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> geometry 'Point(1 1 1)', 6);

SELECT round(tgeompoint 'Point(1 1 1)@2000-01-01' <-> geometry 'Point Z empty', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> geometry 'Point Z empty', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> geometry 'Point Z empty', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> geometry 'Point Z empty', 6);

SELECT round(tgeogpoint 'Point(1.5 1.5)@2000-01-01' <-> geography 'Point(1 1)', 6);
SELECT round(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <-> geography 'Point(1 1)', 6);
SELECT round(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <-> geography 'Point(1 1)', 6);
SELECT round(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <-> geography 'Point(1 1)', 6);

SELECT round(tgeogpoint 'Point(1.5 1.5)@2000-01-01' <-> geography 'Point empty', 6);
SELECT round(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <-> geography 'Point empty', 6);
SELECT round(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <-> geography 'Point empty', 6);
SELECT round(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <-> geography 'Point empty', 6);

SELECT round(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' <-> geography 'Point(1 1 1)', 6);
SELECT round(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' <-> geography 'Point(1 1 1)', 6);
SELECT round(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' <-> geography 'Point(1 1 1)', 6);
SELECT round(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' <-> geography 'Point(1 1 1)', 6);

SELECT round(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' <-> geography 'Point Z empty', 6);
SELECT round(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' <-> geography 'Point Z empty', 6);
SELECT round(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' <-> geography 'Point Z empty', 6);
SELECT round(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' <-> geography 'Point Z empty', 6);

SELECT round(tgeompoint 'Point(1 1)@2000-01-01' <-> tgeompoint 'Point(2 2)@2000-01-01', 6);
SELECT round(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeompoint 'Point(2 2)@2000-01-01', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeompoint 'Point(2 2)@2000-01-01', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeompoint 'Point(2 2)@2000-01-01', 6);
SELECT round(tgeompoint 'Point(1 1)@2000-01-01' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(tgeompoint 'Point(1 1)@2000-01-01' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(tgeompoint 'Point(1 1)@2000-01-01' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);
SELECT round(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);

SELECT round(tgeompoint 'Point(1 1 1)@2000-01-01' <-> tgeompoint 'Point(2 2 2)@2000-01-01', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeompoint 'Point(2 2 2)@2000-01-01', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeompoint 'Point(2 2 2)@2000-01-01', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeompoint 'Point(2 2 2)@2000-01-01', 6);
SELECT round(tgeompoint 'Point(1 1 1)@2000-01-01' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(tgeompoint 'Point(1 1 1)@2000-01-01' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(tgeompoint 'Point(1 1 1)@2000-01-01' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);

SELECT round(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]' <-> tgeompoint 'Interp=Stepwise;[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02]', 6);

SELECT round(tgeogpoint 'Point(1.5 1.5)@2000-01-01' <-> tgeogpoint 'Point(2.5 2.5)@2000-01-01', 6);
SELECT round(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <-> tgeogpoint 'Point(2.5 2.5)@2000-01-01', 6);
SELECT round(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <-> tgeogpoint 'Point(2.5 2.5)@2000-01-01', 6);
SELECT round(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <-> tgeogpoint 'Point(2.5 2.5)@2000-01-01', 6);
SELECT round(tgeogpoint 'Point(1.5 1.5)@2000-01-01' <-> tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}', 6);
SELECT round(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <-> tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}', 6);
SELECT round(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <-> tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}', 6);
SELECT round(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <-> tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}', 6);
SELECT round(tgeogpoint 'Point(1.5 1.5)@2000-01-01' <-> tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]', 6);
SELECT round(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <-> tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]', 6);
SELECT round(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <-> tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]', 6);
SELECT round(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <-> tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]', 6);
SELECT round(tgeogpoint 'Point(1.5 1.5)@2000-01-01' <-> tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 6);
SELECT round(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <-> tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 6);
SELECT round(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <-> tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 6);
SELECT round(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <-> tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 6);

SELECT round(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' <-> tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01', 6);
SELECT round(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' <-> tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01', 6);
SELECT round(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' <-> tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01', 6);
SELECT round(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' <-> tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01', 6);
SELECT round(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' <-> tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}', 6);
SELECT round(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' <-> tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}', 6);
SELECT round(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' <-> tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}', 6);
SELECT round(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' <-> tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}', 6);
SELECT round(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' <-> tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]', 6);
SELECT round(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' <-> tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]', 6);
SELECT round(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' <-> tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]', 6);
SELECT round(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' <-> tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]', 6);
SELECT round(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' <-> tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 6);
SELECT round(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' <-> tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 6);
SELECT round(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' <-> tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 6);
SELECT round(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' <-> tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 6);

-------------------------------------------------------------------------------

