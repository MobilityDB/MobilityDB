-------------------------------------------------------------------------------
-- Discrete Frechet distance
-------------------------------------------------------------------------------

SELECT frechetDistance(tint '1@2001-01-01', tint '1@2001-01-01');
SELECT frechetDistance(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '1@2001-01-01');
SELECT frechetDistance(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '1@2001-01-01');
SELECT frechetDistance(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '1@2001-01-01');
SELECT frechetDistance(tint '1@2001-01-01', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT frechetDistance(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT frechetDistance(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT frechetDistance(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT frechetDistance(tint '1@2001-01-01', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT frechetDistance(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT frechetDistance(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT frechetDistance(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT frechetDistance(tint '1@2001-01-01', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');
SELECT frechetDistance(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');
SELECT frechetDistance(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');
SELECT frechetDistance(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');

SELECT frechetDistance(tbigint '1@2001-01-01', tbigint '1@2001-01-01');
SELECT frechetDistance(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '1@2001-01-01');
SELECT frechetDistance(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '1@2001-01-01');
SELECT frechetDistance(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '1@2001-01-01');
SELECT frechetDistance(tbigint '1@2001-01-01', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT frechetDistance(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT frechetDistance(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT frechetDistance(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT frechetDistance(tbigint '1@2001-01-01', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT frechetDistance(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT frechetDistance(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT frechetDistance(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT frechetDistance(tbigint '1@2001-01-01', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');
SELECT frechetDistance(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');
SELECT frechetDistance(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');
SELECT frechetDistance(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');

SELECT frechetDistance(tfloat '1.5@2001-01-01', tfloat '1.5@2001-01-01');
SELECT frechetDistance(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '1.5@2001-01-01');
SELECT frechetDistance(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '1.5@2001-01-01');
SELECT frechetDistance(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '1.5@2001-01-01');
SELECT frechetDistance(tfloat '1.5@2001-01-01', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}');
SELECT frechetDistance(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}');
SELECT frechetDistance(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}');
SELECT frechetDistance(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}');
SELECT frechetDistance(tfloat '1.5@2001-01-01', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]');
SELECT frechetDistance(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]');
SELECT frechetDistance(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]');
SELECT frechetDistance(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]');
SELECT frechetDistance(tfloat '1.5@2001-01-01', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}');
SELECT frechetDistance(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}');
SELECT frechetDistance(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}');
SELECT frechetDistance(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}');

-------------------------------------------------------------------------------
-- Discrete Frechet distance path
-------------------------------------------------------------------------------

WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2001-01-01', tint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2001-01-01', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2001-01-01', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2001-01-01', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;

WITH Temp AS (
  SELECT frechetDistancePath(tbigint '1@2001-01-01', tbigint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '1@2001-01-01', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '1@2001-01-01', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '1@2001-01-01', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;

WITH Temp AS (
  SELECT frechetDistancePath(tfloat '1.5@2001-01-01', tfloat '1.5@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '1.5@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '1.5@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '1.5@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '1.5@2001-01-01', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '1.5@2001-01-01', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '1.5@2001-01-01', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;

-------------------------------------------------------------------------------
-- Dynamic Time Warp (DTW) distance
-------------------------------------------------------------------------------

SELECT dynTimeWarpDistance(tint '1@2001-01-01', tint '1@2001-01-01');
SELECT dynTimeWarpDistance(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '1@2001-01-01');
SELECT dynTimeWarpDistance(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '1@2001-01-01');
SELECT dynTimeWarpDistance(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '1@2001-01-01');
SELECT dynTimeWarpDistance(tint '1@2001-01-01', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT dynTimeWarpDistance(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT dynTimeWarpDistance(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT dynTimeWarpDistance(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT dynTimeWarpDistance(tint '1@2001-01-01', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT dynTimeWarpDistance(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT dynTimeWarpDistance(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT dynTimeWarpDistance(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT dynTimeWarpDistance(tint '1@2001-01-01', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');
SELECT dynTimeWarpDistance(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');
SELECT dynTimeWarpDistance(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');
SELECT dynTimeWarpDistance(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');

SELECT dynTimeWarpDistance(tbigint '1@2001-01-01', tbigint '1@2001-01-01');
SELECT dynTimeWarpDistance(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '1@2001-01-01');
SELECT dynTimeWarpDistance(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '1@2001-01-01');
SELECT dynTimeWarpDistance(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '1@2001-01-01');
SELECT dynTimeWarpDistance(tbigint '1@2001-01-01', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT dynTimeWarpDistance(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT dynTimeWarpDistance(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT dynTimeWarpDistance(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}');
SELECT dynTimeWarpDistance(tbigint '1@2001-01-01', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT dynTimeWarpDistance(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT dynTimeWarpDistance(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT dynTimeWarpDistance(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT dynTimeWarpDistance(tbigint '1@2001-01-01', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');
SELECT dynTimeWarpDistance(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');
SELECT dynTimeWarpDistance(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');
SELECT dynTimeWarpDistance(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}');

SELECT dynTimeWarpDistance(tfloat '1.5@2001-01-01', tfloat '1.5@2001-01-01');
SELECT dynTimeWarpDistance(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '1.5@2001-01-01');
SELECT dynTimeWarpDistance(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '1.5@2001-01-01');
SELECT dynTimeWarpDistance(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '1.5@2001-01-01');
SELECT dynTimeWarpDistance(tfloat '1.5@2001-01-01', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}');
SELECT dynTimeWarpDistance(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}');
SELECT dynTimeWarpDistance(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}');
SELECT dynTimeWarpDistance(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}');
SELECT dynTimeWarpDistance(tfloat '1.5@2001-01-01', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]');
SELECT dynTimeWarpDistance(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]');
SELECT dynTimeWarpDistance(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]');
SELECT dynTimeWarpDistance(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]');
SELECT dynTimeWarpDistance(tfloat '1.5@2001-01-01', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}');
SELECT dynTimeWarpDistance(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}');
SELECT dynTimeWarpDistance(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}');
SELECT dynTimeWarpDistance(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}');

-------------------------------------------------------------------------------
-- Dynamic Time Warp (DTW) path
-------------------------------------------------------------------------------

WITH Temp AS (
  SELECT dynTimeWarpPath(tint '1@2001-01-01', tint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '1@2001-01-01', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '1@2001-01-01', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '1@2001-01-01', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;

WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '1@2001-01-01', tbigint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '1@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '1@2001-01-01', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '1@2001-01-01', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '1@2001-01-01', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;

WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '1.5@2001-01-01', tfloat '1.5@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '1.5@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '1.5@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '1.5@2001-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '1.5@2001-01-01', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '1.5@2001-01-01', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '1.5@2001-01-01', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynTimeWarpPath(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}') )
SELECT COUNT(*) FROM Temp;

-------------------------------------------------------------------------------
-- Hausdorff, average Hausdorff and LCSS distances
-------------------------------------------------------------------------------

SELECT hausdorffDistance(tint '[1@2001-01-01, 3@2001-01-02, 2@2001-01-03]', tint '[2@2001-01-01, 5@2001-01-02, 4@2001-01-03]');
SELECT hausdorffDistance(tbigint '[1@2001-01-01, 3@2001-01-02, 2@2001-01-03]', tbigint '[2@2001-01-01, 5@2001-01-02, 4@2001-01-03]');
SELECT hausdorffDistance(tfloat '[1@2001-01-01, 3@2001-01-02, 2@2001-01-03]', tfloat '[2@2001-01-01, 5@2001-01-02, 4@2001-01-03]');

SELECT round(averageHausdorffDistance(tint '[1@2001-01-01, 3@2001-01-02, 2@2001-01-03]', tint '[2@2001-01-01, 5@2001-01-02, 4@2001-01-03]')::numeric, 6);
SELECT round(averageHausdorffDistance(tbigint '[1@2001-01-01, 3@2001-01-02, 2@2001-01-03]', tbigint '[2@2001-01-01, 5@2001-01-02, 4@2001-01-03]')::numeric, 6);
SELECT round(averageHausdorffDistance(tfloat '[1@2001-01-01, 3@2001-01-02, 2@2001-01-03]', tfloat '[2@2001-01-01, 5@2001-01-02, 4@2001-01-03]')::numeric, 6);

SELECT round(lcssDistance(tint '[1@2001-01-01, 3@2001-01-02, 2@2001-01-03]', tint '[2@2001-01-01, 5@2001-01-02, 4@2001-01-03]', 1.0)::numeric, 6);
SELECT round(lcssDistance(tbigint '[1@2001-01-01, 3@2001-01-02, 2@2001-01-03]', tbigint '[2@2001-01-01, 5@2001-01-02, 4@2001-01-03]', 1.0)::numeric, 6);
SELECT round(lcssDistance(tfloat '[1@2001-01-01, 3@2001-01-02, 2@2001-01-03]', tfloat '[2@2001-01-01, 5@2001-01-02, 4@2001-01-03]', 1.0)::numeric, 6);

-- Two big integers keep their difference beyond the 53 bits of a float mantissa
SELECT frechetDistance(tbigint '1152921504606846977@2001-01-01', tbigint '1152921504606846976@2001-01-01');
SELECT dynTimeWarpDistance(tbigint '1152921504606846977@2001-01-01', tbigint '1152921504606846976@2001-01-01');
SELECT hausdorffDistance(tbigint '1152921504606846977@2001-01-01', tbigint '1152921504606846976@2001-01-01');
SELECT averageHausdorffDistance(tbigint '1152921504606846977@2001-01-01', tbigint '1152921504606846976@2001-01-01');
SELECT lcssDistance(tbigint '1152921504606846977@2001-01-01', tbigint '1152921504606846976@2001-01-01', 0.5);
SELECT hausdorffDistance(tbigint '9223372036854775806@2001-01-01', tbigint '-9223372036854775808@2001-01-01') = 2::float ^ 64;

-------------------------------------------------------------------------------
