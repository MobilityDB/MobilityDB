-------------------------------------------------------------------------------
-- Discrete Frechet distance
-------------------------------------------------------------------------------

SELECT round(frechetDistance(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint 'Point(1 1)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint 'Point(1 1)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint 'Point(1 1)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint 'Point(1 1)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);
SELECT round(frechetDistance(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);
SELECT round(frechetDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);
SELECT round(frechetDistance(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);

-- 3D
SELECT round(frechetDistance(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint 'Point(1 1 1)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint 'Point(1 1 1)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint 'Point(1 1 1)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint 'Point(1 1 1)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);
SELECT round(frechetDistance(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);
SELECT round(frechetDistance(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);
SELECT round(frechetDistance(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);

-- Geography

SELECT round(frechetDistance(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint 'Point(1.5 1.5)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint 'Point(1.5 1.5)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint 'Point(1.5 1.5)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint 'Point(1.5 1.5)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}'), 6);
SELECT round(frechetDistance(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}'), 6);
SELECT round(frechetDistance(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}'), 6);
SELECT round(frechetDistance(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}'), 6);

--3D
SELECT round(frechetDistance(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01'), 6);
SELECT round(frechetDistance(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}'), 6);
SELECT round(frechetDistance(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]'), 6);
SELECT round(frechetDistance(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}'), 6);
SELECT round(frechetDistance(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}'), 6);
SELECT round(frechetDistance(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}'), 6);
SELECT round(frechetDistance(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}'), 6);

-------------------------------------------------------------------------------
-- Discrete Frechet path
-------------------------------------------------------------------------------

WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint 'Point(1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint 'Point(1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint 'Point(1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint 'Point(1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;

-- 3D
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint 'Point(1 1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint 'Point(1 1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint 'Point(1 1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint 'Point(1 1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;

-- Geography

WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint 'Point(1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint 'Point(1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint 'Point(1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint 'Point(1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;

--3D
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;

-------------------------------------------------------------------------------
-- Dynamic Time Warp distance
-------------------------------------------------------------------------------

SELECT round(dynTimeWarpDistance(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint 'Point(1 1)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint 'Point(1 1)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint 'Point(1 1)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint 'Point(1 1)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}'), 6);

-- 3D
SELECT round(dynTimeWarpDistance(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint 'Point(1 1 1)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint 'Point(1 1 1)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint 'Point(1 1 1)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint 'Point(1 1 1)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);
SELECT round(dynTimeWarpDistance(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}'), 6);

-- Geography

SELECT round(dynTimeWarpDistance(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint 'Point(1.5 1.5)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint 'Point(1.5 1.5)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint 'Point(1.5 1.5)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint 'Point(1.5 1.5)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}'), 6);

--3D
SELECT round(dynTimeWarpDistance(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}'), 6);
SELECT round(dynTimeWarpDistance(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}'), 6);

-------------------------------------------------------------------------------
-- Dynamic Time Warp path
-------------------------------------------------------------------------------

WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint 'Point(1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint 'Point(1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint 'Point(1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint 'Point(1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint 'Point(1 1)@2001-01-01', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;

-- 3D
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint 'Point(1 1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint 'Point(1 1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint 'Point(1 1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint 'Point(1 1 1)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint 'Point(1 1 1)@2001-01-01', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;

-- Geography

WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint 'Point(1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint 'Point(1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint 'Point(1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint 'Point(1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint 'Point(1.5 1.5)@2001-01-01', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}', tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;

--3D
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}', tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;

--------------------------------------------------------
-- One degree of meridian near the equator measures about 110575 m on the spheroid and 111195 m on the sphere
SELECT round(frechetDistance(tgeogpoint '[Point(0 0)@2001-01-01, Point(0 1)@2001-01-02]', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]'), 3);
SELECT round(frechetDistance(tgeogpoint '[Point(0 0)@2001-01-01, Point(0 1)@2001-01-02]', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', false), 3);
SELECT round(dynTimeWarpDistance(tgeogpoint '[Point(0 0)@2001-01-01, Point(0 1)@2001-01-02]', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]'), 3);
SELECT round(dynTimeWarpDistance(tgeogpoint '[Point(0 0)@2001-01-01, Point(0 1)@2001-01-02]', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', false), 3);
SELECT frechetDistancePath(tgeogpoint '[Point(0 0)@2001-01-01, Point(0 1)@2001-01-02]', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', false);
SELECT dynTimeWarpPath(tgeogpoint '[Point(0 0)@2001-01-01, Point(0 1)@2001-01-02]', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', false);
