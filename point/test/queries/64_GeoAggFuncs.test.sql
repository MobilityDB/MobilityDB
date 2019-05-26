-------------------------------------------------------------------------------

SELECT astext(tcentroid(temp)) FROM (VALUES 
  (tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02)'),
  (tgeompoint '[Point(3 3)@2000-01-03, Point(4 4)@2000-01-04)'),
  (tgeompoint '[Point(2 2)@2000-01-02, Point(3 3)@2000-01-03)')) t(temp);
SELECT astext(tcentroid(temp)) FROM (VALUES 
  (tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02)'),
  (tgeompoint '[Point(3 3 3)@2000-01-03, Point(4 4 4)@2000-01-04)'),
  (tgeompoint '[Point(2 2 2)@2000-01-02, Point(3 3 3)@2000-01-03)')) t(temp);

/* Errors */
SELECT astext(tcentroid(temp)) FROM (VALUES 
  (tgeompoint 'Point(0 0)@2000-01-01'),
  (tgeompoint 'srid=5676;Point(1 1)@2000-01-01'),
  ('Point(2 2)@2000-01-01')) t(temp);
SELECT astext(tcentroid(temp)) FROM (VALUES 
  (tgeompoint 'Point(0 0)@2000-01-01'),
  (tgeompoint 'Point(1 1)@2000-01-01'),
  ('Point(2 2 2)@2000-01-01')) t(temp);
SELECT astext(tcentroid(temp)) FROM (VALUES 
  (tgeompoint 'Point(0 0)@2000-01-01'),
  (tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]'),
  ('Point(2 2 2)@2000-01-01')) t(temp);

-------------------------------------------------------------------------------
