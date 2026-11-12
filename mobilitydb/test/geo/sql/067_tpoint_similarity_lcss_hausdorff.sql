-- Coverage fixtures for lcssDistance and averageHausdorffDistance. Targets
-- the inner DP loop and both branches of the match-vs-skip recurrence in
-- tinstarr_lcss_distance, and both directions of the directed Hausdorff in
-- tinstarr_average_hausdorff_distance.

-------------------------------------------------------------------------------
-- LCSS distance — tgeompoint
-------------------------------------------------------------------------------

-- Identical trajectories at eps=0 → LCSS == count, distance index high
SELECT round(lcssDistance(
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]',
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]',
  0.0)::numeric, 6);

-- Identical trajectories at large eps → still same
SELECT round(lcssDistance(
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]',
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]',
  10.0)::numeric, 6);

-- Disjoint trajectories at small eps → no matches → LCSS == 0
SELECT round(lcssDistance(
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]',
  tgeompoint '[Point(100 100)@2001-01-01, Point(101 101)@2001-01-02]',
  0.5)::numeric, 6);

-- Disjoint trajectories at huge eps → all match → LCSS == min(count1,count2)
SELECT round(lcssDistance(
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]',
  tgeompoint '[Point(100 100)@2001-01-01, Point(101 101)@2001-01-02]',
  500.0)::numeric, 6);

-- Asymmetric lengths — exercises the count1 > count2 ternary swap
SELECT round(lcssDistance(
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03, Point(3 3)@2001-01-04, Point(4 4)@2001-01-05]',
  tgeompoint '[Point(0 0)@2001-01-01, Point(2 2)@2001-01-02]',
  0.5)::numeric, 6);

SELECT round(lcssDistance(
  tgeompoint '[Point(0 0)@2001-01-01, Point(2 2)@2001-01-02]',
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03, Point(3 3)@2001-01-04, Point(4 4)@2001-01-05]',
  0.5)::numeric, 6);

-- Partial overlap — forces the Max(prev[j], curr[j-1]) skip branch
SELECT round(lcssDistance(
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02, Point(5 5)@2001-01-03, Point(6 6)@2001-01-04]',
  tgeompoint '[Point(0 0)@2001-01-01, Point(2 2)@2001-01-02, Point(5 5)@2001-01-03, Point(7 7)@2001-01-04]',
  0.5)::numeric, 6);

-------------------------------------------------------------------------------
-- LCSS distance — tgeogpoint
-------------------------------------------------------------------------------

SELECT round(lcssDistance(
  tgeogpoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]',
  tgeogpoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]',
  0.001)::numeric, 6);

-------------------------------------------------------------------------------
-- LCSS distance — error paths (defensive validation)
-------------------------------------------------------------------------------

-- Negative epsilon → returns NULL
SELECT lcssDistance(
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]',
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]',
  -1.0);

-------------------------------------------------------------------------------
-- Average Hausdorff distance — tgeompoint
-------------------------------------------------------------------------------

-- Identical → distance == 0
SELECT round(averageHausdorffDistance(
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]',
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]')::numeric, 6);

-- Asymmetric — directed-pair inner loops, both directions
SELECT round(averageHausdorffDistance(
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03, Point(3 3)@2001-01-04]',
  tgeompoint '[Point(0 0)@2001-01-01, Point(3 3)@2001-01-02]')::numeric, 6);

SELECT round(averageHausdorffDistance(
  tgeompoint '[Point(0 0)@2001-01-01, Point(3 3)@2001-01-02]',
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03, Point(3 3)@2001-01-04]')::numeric, 6);

-- One trajectory is a prefix of the other
SELECT round(averageHausdorffDistance(
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]',
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]')::numeric, 6);

-- Disjoint constant-offset trajectories → distance == offset magnitude
SELECT round(averageHausdorffDistance(
  tgeompoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]',
  tgeompoint '[Point(10 0)@2001-01-01, Point(11 1)@2001-01-02]')::numeric, 6);

-------------------------------------------------------------------------------
-- Average Hausdorff distance — tgeogpoint
-------------------------------------------------------------------------------

SELECT round(averageHausdorffDistance(
  tgeogpoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]',
  tgeogpoint '[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]')::numeric, 6);

-------------------------------------------------------------------------------

-- One degree of meridian near the equator measures about 110575 m on the spheroid and 111195 m on the sphere
SELECT round(hausdorffDistance(tgeogpoint '[Point(0 0)@2001-01-01, Point(0 1)@2001-01-02]', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]')::numeric, 3);
SELECT round(hausdorffDistance(tgeogpoint '[Point(0 0)@2001-01-01, Point(0 1)@2001-01-02]', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', false)::numeric, 3);
SELECT round(averageHausdorffDistance(tgeogpoint '[Point(0 0)@2001-01-01, Point(0 1)@2001-01-02]', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]')::numeric, 3);
SELECT round(averageHausdorffDistance(tgeogpoint '[Point(0 0)@2001-01-01, Point(0 1)@2001-01-02]', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', false)::numeric, 3);
-- Within 110800 m both pairs of consecutive meridian points match on the spheroid, only the shared point on the sphere
SELECT round(lcssDistance(tgeogpoint '[Point(0 0)@2001-01-01, Point(0 1)@2001-01-02]', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', 110800)::numeric, 6);
SELECT round(lcssDistance(tgeogpoint '[Point(0 0)@2001-01-01, Point(0 1)@2001-01-02]', tgeogpoint '[Point(0 1)@2001-01-01, Point(0 2)@2001-01-02]', 110800, false)::numeric, 6);

-------------------------------------------------------------------------------

-- Values in two reference systems are not comparable
SELECT frechetDistance(tgeompoint 'SRID=4326;[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]', tgeompoint 'SRID=3857;[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]');
SELECT dynTimeWarpDistance(tgeompoint 'SRID=4326;[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]', tgeompoint 'SRID=3857;[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]');
SELECT hausdorffDistance(tgeompoint 'SRID=4326;[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]', tgeompoint 'SRID=3857;[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]');
SELECT averageHausdorffDistance(tgeompoint 'SRID=4326;[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]', tgeompoint 'SRID=3857;[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]');
SELECT lcssDistance(tgeompoint 'SRID=4326;[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]', tgeompoint 'SRID=3857;[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]', 1.0);
SELECT frechetDistancePath(tgeompoint 'SRID=4326;[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]', tgeompoint 'SRID=3857;[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]');
SELECT dynTimeWarpPath(tgeompoint 'SRID=4326;[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]', tgeompoint 'SRID=3857;[Point(0 0)@2001-01-01, Point(1 1)@2001-01-02]');

-------------------------------------------------------------------------------

-- A value with no Z has any Z, so the distance is the 2D one in either order
SELECT round(frechetDistance(tgeompoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]', tgeompoint '[Point(3 4 12)@2001-01-01, Point(3 4 12)@2001-01-02]')::numeric, 6);
SELECT round(frechetDistance(tgeompoint '[Point(3 4 12)@2001-01-01, Point(3 4 12)@2001-01-02]', tgeompoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]')::numeric, 6);
SELECT round(dynTimeWarpDistance(tgeompoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]', tgeompoint '[Point(3 4 12)@2001-01-01, Point(3 4 12)@2001-01-02]')::numeric, 6);
SELECT round(dynTimeWarpDistance(tgeompoint '[Point(3 4 12)@2001-01-01, Point(3 4 12)@2001-01-02]', tgeompoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]')::numeric, 6);
SELECT round(hausdorffDistance(tgeompoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]', tgeompoint '[Point(3 4 12)@2001-01-01, Point(3 4 12)@2001-01-02]')::numeric, 6);
SELECT round(hausdorffDistance(tgeompoint '[Point(3 4 12)@2001-01-01, Point(3 4 12)@2001-01-02]', tgeompoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]')::numeric, 6);
SELECT round(averageHausdorffDistance(tgeompoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]', tgeompoint '[Point(3 4 12)@2001-01-01, Point(3 4 12)@2001-01-02]')::numeric, 6);
SELECT round(averageHausdorffDistance(tgeompoint '[Point(3 4 12)@2001-01-01, Point(3 4 12)@2001-01-02]', tgeompoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]')::numeric, 6);
SELECT round(lcssDistance(tgeompoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]', tgeompoint '[Point(3 4 12)@2001-01-01, Point(3 4 12)@2001-01-02]', 6)::numeric, 6);
SELECT round(lcssDistance(tgeompoint '[Point(3 4 12)@2001-01-01, Point(3 4 12)@2001-01-02]', tgeompoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]', 6)::numeric, 6);

-------------------------------------------------------------------------------
