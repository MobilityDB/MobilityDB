-------------------------------------------------------------------------------
-- Tiling for temporal rigid geometries
-------------------------------------------------------------------------------

SELECT array_length(spaceBoxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]', 2.0), 1);
SELECT array_length(spaceBoxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]', 2.0, 2.0), 1);
SELECT array_length(spaceBoxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]', 2.0, 2.0, 2.0), 1);

SELECT array_length(timeBoxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-05]', interval '1 day'), 1);

SELECT array_length(spaceTimeBoxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-05]', 2.0, interval '1 day'), 1);
SELECT array_length(spaceTimeBoxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-05]', 2.0, 2.0, interval '1 day'), 1);
SELECT array_length(spaceTimeBoxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-05]', 2.0, 2.0, 2.0, interval '1 day'), 1);

-------------------------------------------------------------------------------
