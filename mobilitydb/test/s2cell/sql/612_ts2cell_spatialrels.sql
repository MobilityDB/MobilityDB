-------------------------------------------------------------------------------

-- Ever and always spatial relationships between a ts2cell and a geography,
-- or two ts2cell values.
--
-- An S2 cell is a region of the sphere bounded by great-circle arcs, so each
-- relationship is the one of the temporal geography of the cell boundary,
-- answered on the sphere; the answer must equal the same relationship asked
-- of cellToBoundary directly, and a distance is in metres.

-------------------------------------------------------------------------------
-- The delegation identity
-------------------------------------------------------------------------------

SELECT eIntersects(ts2cell '[47c3c3@2001-01-01]', geography 'Point(4.35 50.85)')
  = eIntersects(cellToBoundary(ts2cell '[47c3c3@2001-01-01]'),
      geography 'Point(4.35 50.85)');
SELECT aIntersects(geography 'Point(4.35 50.85)', ts2cell '[47c3c3@2001-01-01]')
  = aIntersects(geography 'Point(4.35 50.85)',
      cellToBoundary(ts2cell '[47c3c3@2001-01-01]'));

-------------------------------------------------------------------------------
-- A point inside its own cell, and one far away
-------------------------------------------------------------------------------

SELECT eIntersects(ts2cell '[47c3c3@2001-01-01]', geography 'Point(4.35 50.85)');
SELECT eIntersects(ts2cell '[47c3c3@2001-01-01]', geography 'Point(-122.4 37.8)');
SELECT eDisjoint(ts2cell '[47c3c3@2001-01-01]', geography 'Point(-122.4 37.8)');
SELECT eDwithin(ts2cell '[47c3c3@2001-01-01]', geography 'Point(4.35 50.85)', 1000.0);
SELECT aDwithin(ts2cell '[47c3c3@2001-01-01]', ts2cell '[47c3c3@2001-01-01]', 1.0);

-------------------------------------------------------------------------------
-- The static operand first, and two temporal cells
-------------------------------------------------------------------------------

SELECT eIntersects(geography 'Point(4.35 50.85)', ts2cell '[47c3c3@2001-01-01]');
SELECT eIntersects(geography 'Point(-122.4 37.8)', ts2cell '[47c3c3@2001-01-01]');
SELECT eDwithin(geography 'Point(4.35 50.85)', ts2cell '[47c3c3@2001-01-01]', 1000.0);
SELECT eIntersects(ts2cell '[47c3c3@2001-01-01, 47c3c3@2001-01-02]',
  ts2cell '[47c3c3@2001-01-01]');
SELECT eDwithin(ts2cell '[47c3c3@2001-01-01, 47c3c3@2001-01-02]',
  ts2cell '[47c3c3@2001-01-01]', 1000.0);

-------------------------------------------------------------------------------
-- Two temporal cells over disjoint times have no instant to answer on, and
-- the relationship is NULL; an empty geography is ever reached by nothing
-------------------------------------------------------------------------------

SELECT eIntersects(ts2cell '[47c3c3@2001-01-01]', ts2cell '[47c3c3@2001-01-02]');
SELECT eDwithin(ts2cell '[47c3c3@2001-01-01]', ts2cell '[47c3c3@2001-01-02]', 1000.0);
SELECT eIntersects(geography 'Point EMPTY', ts2cell '[47c3c3@2001-01-01]');
SELECT eIntersects(ts2cell '[47c3c3@2001-01-01]', geography 'Point EMPTY');
SELECT eDwithin(geography 'Point EMPTY', ts2cell '[47c3c3@2001-01-01]', 1000.0);
SELECT eDwithin(ts2cell '[47c3c3@2001-01-01]', geography 'Point EMPTY', 1000.0);

-------------------------------------------------------------------------------
-- The cell is the region S2 assigns its points to. The first point lies in
-- the level-2 cell of the queries below and 1,220 km outside the polygon
-- joining its vertices with straight lines in longitude and latitude; the
-- second lies 397 km inside that polygon and outside the cell.
-------------------------------------------------------------------------------

SELECT geoToS2Cell(geography 'Point(88.8843 79.0319)', 2) = s2cell '45',
  eIntersects(ts2cell '[45@2001-01-01]', geography 'Point(88.8843 79.0319)');
SELECT geoToS2Cell(geography 'Point(-0.3719 71.2156)', 2) = s2cell '45',
  eIntersects(ts2cell '[45@2001-01-01]', geography 'Point(-0.3719 71.2156)');

-------------------------------------------------------------------------------
