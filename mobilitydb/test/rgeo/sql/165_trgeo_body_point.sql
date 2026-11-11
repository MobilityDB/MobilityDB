-- bodyPointTrajectory(trgeometry, geometry) → tgeompoint:
-- world-frame trajectory of an arbitrary body-frame point on a moving rigid
-- geometry. The reference-point case (body point = origin) must agree with
-- the existing trgeometry::tgeompoint cast.

-------------------------------------------------------------------------------
-- 2D: a unit-square body translates from origin to (10,0) while yawing 90°.
-------------------------------------------------------------------------------

-- Body origin (0,0) trajectory must match the reference cast.
SELECT asText(round(bodyPointTrajectory(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),1.5707963267948966)@2026-01-01 00:01:00]',
  'Point(0 0)'), 6));

SELECT asText((trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),1.5707963267948966)@2026-01-01 00:01:00]')::tgeompoint);
SELECT bodyPointTrajectory(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),1.5707963267948966)@2026-01-01 00:01:00]',
  'Point(0 0)') =
  (trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),1.5707963267948966)@2026-01-01 00:01:00]')::tgeompoint;

-- Body front-right corner (1,0): at t1 (yaw=0) world (1,0); at t2 (yaw=π/2)
-- the body's +X axis now points +Y, so the corner is at world (10,1).
SELECT asText(round(bodyPointTrajectory(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),1.5707963267948966)@2026-01-01 00:01:00]',
  'Point(1 0)'), 6));

-- Body back-left corner (0,1): at t1 world (0,1); at t2 world (9,0) (the
-- corner now extends in the body's -Y direction, which is -X in the world).
SELECT asText(round(bodyPointTrajectory(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),1.5707963267948966)@2026-01-01 00:01:00]',
  'Point(0 1)'), 6));

-- A point off the center of a body that only rotates follows a quarter of a
-- circle, which the linear result replaces by the chord between its positions
SELECT asText(round(bodyPointTrajectory(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(0 0),1.5707963267948966)@2026-01-01 00:01:00]',
  'Point(1 0)'), 6));

-- A body with step interpolation gives a trajectory with step interpolation
SELECT asText(round(bodyPointTrajectory(
  trgeometry 'Interp=Step;Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),1.5707963267948966)@2026-01-01 00:01:00]',
  'Point(1 0)'), 6));

-------------------------------------------------------------------------------
-- 2D static body (no motion): trajectory is constant.
-------------------------------------------------------------------------------

SELECT asText(round(bodyPointTrajectory(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(5 5),0)@2026-01-01, Pose(Point(5 5),0)@2026-01-01 00:01:00]',
  'Point(2 3)'), 6));

-------------------------------------------------------------------------------
-- Errors
-------------------------------------------------------------------------------

/* Errors */

-- Body argument must be a point.
SELECT bodyPointTrajectory(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),0)@2026-01-01 00:01:00]',
  'Linestring(0 0, 1 1)');

-- Body-point dimensionality must match the trgeometry's pose.
SELECT bodyPointTrajectory(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),0)@2026-01-01 00:01:00]',
  'PointZ(0 0 1)');

-------------------------------------------------------------------------------
