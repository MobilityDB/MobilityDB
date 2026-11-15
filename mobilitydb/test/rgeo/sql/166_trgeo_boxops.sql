-------------------------------------------------------------------------------
-- Bounding box operations for temporal rigid geometries
-------------------------------------------------------------------------------

SELECT array_length(spans(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));{[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-02], [Pose(Point(0 0),0)@2001-01-04, Pose(Point(2 0),0)@2001-01-05]}'), 1);
SELECT array_length(stboxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-02, Pose(Point(10 0),0)@2001-01-03]'), 1);

SELECT array_length(splitNSpans(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-02, Pose(Point(10 0),0)@2001-01-03]', 2), 1);
SELECT array_length(splitEachNSpans(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-02, Pose(Point(10 0),0)@2001-01-03]', 1), 1);
SELECT array_length(splitNStboxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-02, Pose(Point(10 0),0)@2001-01-03]', 2), 1);
SELECT array_length(splitEachNStboxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-02, Pose(Point(10 0),0)@2001-01-03]', 1), 1);
SELECT array_length(mergeNSpans(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-02, Pose(Point(10 0),0)@2001-01-03]', 2), 1);
SELECT array_length(mergeEachNSpans(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-02, Pose(Point(10 0),0)@2001-01-03]', 1), 1);
SELECT array_length(mergeNStboxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-02, Pose(Point(10 0),0)@2001-01-03]', 2), 1);
SELECT array_length(mergeEachNStboxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-02, Pose(Point(10 0),0)@2001-01-03]', 1), 1);

-- A square turning a quarter turn about the origin: at the middle of the
-- segment its corner (2 2) lies at (0 2.83), so the box of the segment holds
-- the geometry at every instant of it, not only at its ends
WITH t(temp) AS (SELECT trgeometry 'Polygon((1 1,2 1,2 2,1 2,1 1));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),1.5707963)@2001-01-02]')
SELECT (stboxes(temp))[1] @> stbox(valueAtTimestamp(temp, '2001-01-01 12:00')),
  (splitNStboxes(temp, 1))[1] @> stbox(valueAtTimestamp(temp, '2001-01-01 12:00')),
  (splitEachNStboxes(temp, 1))[1] @> stbox(valueAtTimestamp(temp, '2001-01-01 12:00'))
FROM t;
WITH t(temp) AS (SELECT trgeometry 'Polygon((1 1,2 1,2 2,1 2,1 1));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),1.5707963)@2001-01-02]')
SELECT (stboxes(temp))[1] @> stbox(valueAtTimestamp(temp, '2001-01-01 12:00')),
  (mergeNStboxes(temp, 1))[1] @> stbox(valueAtTimestamp(temp, '2001-01-01 12:00')),
  (mergeEachNStboxes(temp, 1))[1] @> stbox(valueAtTimestamp(temp, '2001-01-01 12:00'))
FROM t;

-------------------------------------------------------------------------------
