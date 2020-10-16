-------------------------------------------------------------------------------

SELECT asText(tcentroid(temp)) FROM (VALUES
(NULL::tgeompoint),('Point(1 1)@2000-01-01'::tgeompoint),(NULL::tgeompoint)) t(temp);

SELECT asText(tcentroid(temp)) FROM (VALUES 
  (tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02)'),
  (tgeompoint '[Point(3 3)@2000-01-03, Point(4 4)@2000-01-04)'),
  (tgeompoint '[Point(2 2)@2000-01-02, Point(3 3)@2000-01-03)')) t(temp);
SELECT asText(tcentroid(temp)) FROM (VALUES 
  (tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02)'),
  (tgeompoint '[Point(3 3 3)@2000-01-03, Point(4 4 4)@2000-01-04)'),
  (tgeompoint '[Point(2 2 2)@2000-01-02, Point(3 3 3)@2000-01-03)')) t(temp);

/* Errors */
SELECT asText(tcentroid(temp)) FROM (VALUES 
  (tgeompoint 'Point(0 0)@2000-01-01'),
  (tgeompoint 'srid=5676;Point(1 1)@2000-01-01'),
  ('Point(2 2)@2000-01-01')) t(temp);
SELECT asText(tcentroid(temp)) FROM (VALUES 
  (tgeompoint 'Point(0 0)@2000-01-01'),
  (tgeompoint 'Point(1 1)@2000-01-01'),
  ('Point(2 2 2)@2000-01-01')) t(temp);
SELECT asText(tcentroid(temp)) FROM (VALUES 
  (tgeompoint 'Point(0 0)@2000-01-01'),
  (tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02}'),
  ('Point(2 2 2)@2000-01-01')) t(temp);
SELECT asText(tcentroid(temp)) FROM (VALUES 
  (tgeompoint 'Point(0 0)@2000-01-01'),
  (tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]'),
  ('Point(2 2 2)@2000-01-01')) t(temp);
SELECT asText(tcentroid(temp)) FROM (VALUES 
  (tgeompoint '[Point(0 0)@2000-01-01]'),
  (tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]'),
  ('Point(2 2 2)@2000-01-01')) t(temp);
  
-------------------------------------------------------------------------------

SELECT extent(temp) FROM (VALUES
(NULL::tgeompoint),('Point(1 1)@2000-01-01'::tgeompoint),(NULL::tgeompoint)) t(temp);

SELECT extent(temp) FROM (VALUES
  (tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02)'),
  (tgeompoint '[Point(3 3)@2000-01-03, Point(4 4)@2000-01-04)'),
  (tgeompoint '[Point(2 2)@2000-01-02, Point(3 3)@2000-01-03)')) t(temp);
SELECT setprecision(extent(temp), 13) FROM (VALUES
  (tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02)'),
  (tgeogpoint '[Point(3 3 3)@2000-01-03, Point(4 4 4)@2000-01-04)'),
  (tgeogpoint '[Point(2 2 2)@2000-01-02, Point(3 3 3)@2000-01-03)')) t(temp);

/* Errors */
SELECT extent(temp) FROM (VALUES
  (tgeompoint 'Point(1 1 1)@2000-01-01'),
  (tgeompoint 'Point(1 1)@2000-01-01')) t(temp);

-------------------------------------------------------------------------------
