-------------------------------------------------------------------------------
/* Errors */
SELECT geometry 'Linestring(1 1,2 2)' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]';
SELECT geometry 'srid=5676;Point(2 2)' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]';
SELECT geometry 'Point(2 2 2)' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> geometry 'Linestring(1 1,2 2)';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> geometry 'srid=5676;Point(2 2)';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> geometry 'Point(2 2 2)';
SELECT tgeompoint 'SRID=5676;[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(3 3 3)@2000-01-03]' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]';


SELECT geometry 'Point(1 1)' <-> tgeompoint 'Point(2 2)@2000-01-01';
SELECT geometry 'Point(1 1)' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}';
SELECT geometry 'Point(1 1)' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]';
SELECT geometry 'Point(2 2)' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]';
SELECT geometry 'Point(1 1)' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT geometry 'Point empty' <-> tgeompoint 'Point(2 2)@2000-01-01';
SELECT geometry 'Point empty' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}';
SELECT geometry 'Point empty' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]';
SELECT geometry 'Point empty' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT geometry 'Point(1 1 1)' <-> tgeompoint 'Point(2 2 2)@2000-01-01';
SELECT geometry 'Point(1 1 1)' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}';
SELECT geometry 'Point(1 1 1)' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]';
SELECT geometry 'Point(1 1 1)' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT geometry 'Point Z empty' <-> tgeompoint 'Point(2 2 2)@2000-01-01';
SELECT geometry 'Point Z empty' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}';
SELECT geometry 'Point Z empty' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]';
SELECT geometry 'Point Z empty' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT geography 'Point(1 1)' <-> tgeogpoint 'Point(2.5 2.5)@2000-01-01';
SELECT geography 'Point(1 1)' <-> tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}';
SELECT geography 'Point(1 1)' <-> tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]';
SELECT geography 'Point(1 1)' <-> tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT geography 'Point empty' <-> tgeogpoint 'Point(2.5 2.5)@2000-01-01';
SELECT geography 'Point empty' <-> tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}';
SELECT geography 'Point empty' <-> tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]';
SELECT geography 'Point empty' <-> tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT geography 'Point(1 1 1)' <-> tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01';
SELECT geography 'Point(1 1 1)' <-> tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}';
SELECT geography 'Point(1 1 1)' <-> tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]';
SELECT geography 'Point(1 1 1)' <-> tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';

SELECT geography 'Point Z empty' <-> tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01';
SELECT geography 'Point Z empty' <-> tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}';
SELECT geography 'Point Z empty' <-> tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]';
SELECT geography 'Point Z empty' <-> tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <-> geometry 'Point(1 1)';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> geometry 'Point(1 1)';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> geometry 'Point(1 1)';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> geometry 'Point(1 1)';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <-> geometry 'Point empty';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> geometry 'Point empty';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> geometry 'Point empty';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> geometry 'Point empty';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <-> geometry 'Point(1 1 1)';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> geometry 'Point(1 1 1)';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> geometry 'Point(1 1 1)';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> geometry 'Point(1 1 1)';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <-> geometry 'Point Z empty';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> geometry 'Point Z empty';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> geometry 'Point Z empty';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> geometry 'Point Z empty';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <-> geography 'Point(1 1)';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <-> geography 'Point(1 1)';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <-> geography 'Point(1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <-> geography 'Point(1 1)';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <-> geography 'Point empty';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <-> geography 'Point empty';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <-> geography 'Point empty';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <-> geography 'Point empty';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' <-> geography 'Point(1 1 1)';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' <-> geography 'Point(1 1 1)';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' <-> geography 'Point(1 1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' <-> geography 'Point(1 1 1)';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' <-> geography 'Point Z empty';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' <-> geography 'Point Z empty';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' <-> geography 'Point Z empty';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' <-> geography 'Point Z empty';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <-> tgeompoint 'Point(2 2)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeompoint 'Point(2 2)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeompoint 'Point(2 2)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeompoint 'Point(2 2)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <-> tgeompoint 'Point(2 2 2)@2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeompoint 'Point(2 2 2)@2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeompoint 'Point(2 2 2)@2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeompoint 'Point(2 2 2)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <-> tgeogpoint 'Point(2.5 2.5)@2000-01-01';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <-> tgeogpoint 'Point(2.5 2.5)@2000-01-01';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <-> tgeogpoint 'Point(2.5 2.5)@2000-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <-> tgeogpoint 'Point(2.5 2.5)@2000-01-01';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <-> tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <-> tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <-> tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <-> tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <-> tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <-> tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <-> tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <-> tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <-> tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <-> tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <-> tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <-> tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' <-> tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' <-> tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' <-> tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' <-> tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01';
SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' <-> tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' <-> tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' <-> tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' <-> tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' <-> tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' <-> tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' <-> tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' <-> tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]';
SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' <-> tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' <-> tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' <-> tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' <-> tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';

-------------------------------------------------------------------------------

