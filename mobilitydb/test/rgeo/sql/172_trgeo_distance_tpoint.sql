-------------------------------------------------------------------------------
-- Temporal distance between a moving rigid geometry and a temporal geometry
-- point.
--
-- A temporal point does not rotate, so the distance is that of the rigid body
-- against the point placed at the origin after subtracting the point motion
-- from the body pose; the interior turning point of the nearest approach is
-- captured all the same.
-------------------------------------------------------------------------------

-- A long rectangle rotates half a turn next to a point that stays put
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  tgeompoint '[Point(0 3)@2001-01-01, Point(0 3)@2001-01-02]')::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  tgeompoint '[Point(0 3)@2001-01-01, Point(0 3)@2001-01-02]'))::numeric, 6);

-- A long rectangle rotating while a point sweeps across in front of it
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  tgeompoint '[Point(-3 5)@2001-01-01, Point(3 5)@2001-01-02]')::numeric, 6);

-- A rectangle sliding past a point that stays above its path: nearest approach
-- realized midway
SELECT round(startValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  tgeompoint '[Point(5 3)@2001-01-01, Point(5 3)@2001-01-02]'))::numeric, 6);
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  tgeompoint '[Point(5 3)@2001-01-01, Point(5 3)@2001-01-02]')::numeric, 6);

-- The distance operator agrees with the function
SELECT round(minValue(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]' <->
  tgeompoint '[Point(5 3)@2001-01-01, Point(5 3)@2001-01-02]')::numeric, 6);

-- The temporal distance between a moving rigid geometry and a temporal point is
-- linear
SELECT interp(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  tgeompoint '[Point(5 3)@2001-01-01, Point(5 3)@2001-01-02]'));

-- The shortest line joins the body itself and the point
SELECT ST_AsText(shortestLine(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  tgeompoint '[Point(5 3)@2001-01-01, Point(5 3)@2001-01-02]'));

-- The values are synchronized whatever their subtypes: the instants taken out
-- of a rigid geometry carry no reference geometry
SELECT tDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 0));{Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-02}',
  tgeompoint '{Point(10 0)@2001-01-01, Point(10 0)@2001-01-02}');
SELECT tDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 0));{Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-02}',
  tgeompoint '[Point(10 0)@2001-01-01, Point(10 0)@2001-01-02]');
SELECT tDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-02]',
  tgeompoint '{Point(10 0)@2001-01-01, Point(10 0)@2001-01-02}');
SELECT tDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 0));{[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-02],[Pose(Point(5 0),0)@2001-01-03, Pose(Point(6 0),0)@2001-01-04]}',
  tgeompoint '{[Point(10 0)@2001-01-01, Point(10 0)@2001-01-04]}');

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- A temporal rigid geometry in three dimensions has no distance yet, whatever
-- the dimensions of the temporal point
-------------------------------------------------------------------------------

/* Errors */ SELECT tDistance(trgeometry 'Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(5 0 0),1,0,0,0)@2001-01-02]', tgeompoint '[Point(1 1 1)@2001-01-01,Point(1 1 1)@2001-01-02]');
/* Errors */ SELECT tDistance(trgeometry 'Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(5 0 0),1,0,0,0)@2001-01-02]', tgeompoint '[Point(1 1)@2001-01-01,Point(1 1)@2001-01-02]');
/* Errors */ SELECT nearestApproachDistance(trgeometry 'Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(5 0 0),1,0,0,0)@2001-01-02]', tgeompoint '[Point(1 1 1)@2001-01-01,Point(1 1 1)@2001-01-02]');
/* Errors */ SELECT nearestApproachInstant(trgeometry 'Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(5 0 0),1,0,0,0)@2001-01-02]', tgeompoint '[Point(1 1 1)@2001-01-01,Point(1 1 1)@2001-01-02]');
/* Errors */ SELECT shortestLine(trgeometry 'Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(5 0 0),1,0,0,0)@2001-01-02]', tgeompoint '[Point(1 1 1)@2001-01-01,Point(1 1 1)@2001-01-02]');

-------------------------------------------------------------------------------
