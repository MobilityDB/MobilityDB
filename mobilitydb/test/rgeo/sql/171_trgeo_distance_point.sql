-------------------------------------------------------------------------------
-- Temporal distance between a moving rigid geometry and a point geometry.
--
-- The distance of a fixed closest-feature pair curves as the body moves, so
-- the temporal distance captures the interior turning point where the nearest
-- approach happens; for a body passing a point the nearest approach is below
-- the distance at both segment ends.
-------------------------------------------------------------------------------

-- A 2 x 1 rectangle slides past a point three units above its path: the nearest
-- approach is realized midway, below the distance at both ends
SELECT round(startValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Point(5 3)'))::numeric, 6);
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Point(5 3)')::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Point(5 3)'))::numeric, 6);

-- A long rectangle rotating half a turn near a fixed point: the nearest
-- approach happens mid-rotation
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  geometry 'Point(0 3)')::numeric, 6);

-- The distance operator agrees with the function
SELECT round(minValue(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]' <->
  geometry 'Point(5 3)')::numeric, 6);

-- The nearest approach instant lies strictly inside the motion, not at an end
SELECT getTimestamp(nearestApproachInstant(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Point(5 3)')) <@ tstzspan '(2001-01-01, 2001-01-02)';

-------------------------------------------------------------------------------
