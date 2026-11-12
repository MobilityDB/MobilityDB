-- Spatial accessor functions for trgeometry:
--   traversedArea(trgeometry, bool) → geometry
--   centroid(trgeometry) → tgeompoint
--   convexHull(trgeometry) → geometry

-- A unit square translating from (0,0) to (2,0) with no rotation.
-- traversedArea collects the geometry at every pose instant.
SELECT ST_AsText(round(traversedArea(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(2 0),0)@2026-01-02]'), 6));

-- traversedArea with unary union collapses the collection into one geometry.
SELECT ST_AsText(round(traversedArea(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(2 0),0)@2026-01-02]', TRUE), 6));

-- A rod turning 0.04 rad about its centre pivots there, so its top edge
-- crosses itself: no position of the rod reaches (0 0.12), which the hull of
-- its two positions would cover, and its last position covers (1.99 0.15).
SELECT ST_Covers(traversedArea(
  trgeometry 'Polygon((-2 -0.1, 2 -0.1, 2 0.1, -2 0.1, -2 -0.1));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(0 0),0.04)@2026-01-02]', TRUE),
  geometry 'Point(0 0.12)');
SELECT ST_Covers(traversedArea(
  trgeometry 'Polygon((-2 -0.1, 2 -0.1, 2 0.1, -2 0.1, -2 -0.1));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(0 0),0.04)@2026-01-02]', TRUE),
  geometry 'Point(1.99 0.15)');

-- A body that does not interpolate stands at each of its placements and covers
-- nothing between them, so the united form answers the placements themselves
-- and not the corridor between them.
SELECT ST_AsText(round(traversedArea(
  trgeometry 'Interp=Step;Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(2 0),0)@2026-01-02]', TRUE), 6));

-- centroid: trajectory of the polygon centroid under rigid-body motion.
SELECT asText(round(centroid(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(2 0),0)@2026-01-02]'), 6));

-- The centroid keeps the interpolation of the body: halfway through the
-- translation it is halfway between its two positions.
SELECT ST_AsText(valueAtTimestamp(centroid(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(2 0),0)@2026-01-02]'),
  '2026-01-01 12:00'));
SELECT asText(round(centroid(
  trgeometry 'Interp=Step;Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(2 0),0)@2026-01-02]'), 6));

-- convexHull: convex hull of the traversed area.
SELECT ST_AsText(round(convexHull(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(2 0),0)@2026-01-02]'), 6));

-------------------------------------------------------------------------------

-- Motion metrics
SELECT length(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(3 4),0)@2001-01-02]');
SELECT asText(cumulativeLength(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(3 4),0)@2001-01-02]'));
SELECT asText(speed(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(3 4),0)@2001-01-02]'));
SELECT ST_AsText(twCentroid(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(3 4),0)@2001-01-02]'));

-- The shape of a rigid geometry never changes, so its angular speed is the
-- angular speed of the pose that carries it
SELECT asText(round(angularSpeed(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),1)@2001-01-02]'), 9));
SELECT asText(angularSpeed(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),1)@2001-01-02]')) =
  asText(angularSpeed(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),1)@2001-01-02]'));
-- The united form of a body that is not convex, with holes or concave, in
-- linear motion is refused: it sweeps more than its placements and less than
-- their hull
SELECT ST_Area(traversedArea(
  trgeometry 'Polygon((0 0,4 0,4 4,0 4,0 0),(1 1,3 1,3 3,1 3,1 1));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),0)@2026-01-02]', TRUE));
SELECT ST_Area(traversedArea(
  trgeometry 'Polygon((0 0,4 0,4 1,1 1,1 3,0 3,0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),0)@2026-01-02]', TRUE));
-- its placements and its step form answer
SELECT ST_Area(traversedArea(
  trgeometry 'Polygon((0 0,4 0,4 4,0 4,0 0),(1 1,3 1,3 3,1 3,1 1));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),0)@2026-01-02]'));
SELECT ST_Area(traversedArea(
  trgeometry 'Interp=Step;Polygon((0 0,4 0,4 4,0 4,0 0),(1 1,3 1,3 3,1 3,1 1));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),0)@2026-01-02]', TRUE));
-- The united form of a value with Z is the ground it covers, with no Z: a
-- square climbing while it travels covers the corridor of the same square on
-- the ground, a cube the hull of its two shadows, an L and a square with a
-- hole what their faces sweep, the hole of a square moving less than its
-- width staying uncovered, a wall moving along itself a segment
SELECT ST_AsText(round(traversedArea(
  trgeometry 'Polygon Z((0 0 0,1 0 0,1 1 0,0 1 0,0 0 0));[Pose(Point Z(0 0 0),1,0,0,0)@2026-01-01, Pose(Point Z(4 0 3),1,0,0,0)@2026-01-02]', TRUE), 6));
SELECT ST_AsText(round(traversedArea(
  trgeometry 'PolyhedralSurface Z(((0 0 0,0 1 0,1 1 0,1 0 0,0 0 0)),((0 0 1,1 0 1,1 1 1,0 1 1,0 0 1)),((0 0 0,1 0 0,1 0 1,0 0 1,0 0 0)),((1 0 0,1 1 0,1 1 1,1 0 1,1 0 0)),((1 1 0,0 1 0,0 1 1,1 1 1,1 1 0)),((0 1 0,0 0 0,0 0 1,0 1 1,0 1 0)));[Pose(Point Z(0 0 0),1,0,0,0)@2026-01-01, Pose(Point Z(4 2 1),1,0,0,0)@2026-01-02]', TRUE), 6));
SELECT ST_Area(traversedArea(
  trgeometry 'Polygon Z((0 0 0,3 0 0,3 1 0,1 1 0,1 3 0,0 3 0,0 0 0));[Pose(Point Z(0 0 0),1,0,0,0)@2026-01-01, Pose(Point Z(2 2 1),1,0,0,0)@2026-01-02]', TRUE));
SELECT ST_AsText(round(traversedArea(
  trgeometry 'Polygon Z((0 0 0,6 0 0,6 6 0,0 6 0,0 0 0),(2 2 0,4 2 0,4 4 0,2 4 0,2 2 0));[Pose(Point Z(0 0 0),1,0,0,0)@2026-01-01, Pose(Point Z(1 0 0),1,0,0,0)@2026-01-02]', TRUE), 6));
SELECT ST_AsText(round(traversedArea(
  trgeometry 'Polygon Z((0 0 0,1 0 0,1 0 1,0 0 1,0 0 0));[Pose(Point Z(0 0 0),1,0,0,0)@2026-01-01, Pose(Point Z(3 0 0),1,0,0,0)@2026-01-02]', TRUE), 6));
-- in step interpolation, the placements on the ground
SELECT ST_AsText(round(traversedArea(
  trgeometry 'Interp=Step;Polygon Z((0 0 0,1 0 0,1 1 0,0 1 0,0 0 0));[Pose(Point Z(0 0 0),1,0,0,0)@2026-01-01, Pose(Point Z(4 0 3),1,0,0,0)@2026-01-02]', TRUE), 6));
-- a body turning in linear motion is refused, its placements answer
SELECT ST_Area(traversedArea(
  trgeometry 'Polygon Z((0 0 0,1 0 0,1 1 0,0 1 0,0 0 0));[Pose(Point Z(0 0 0),1,0,0,0)@2026-01-01, Pose(Point Z(4 0 0),0,0,0,1)@2026-01-02]', TRUE));
SELECT ST_NumGeometries(traversedArea(
  trgeometry 'Polygon Z((0 0 0,1 0 0,1 1 0,0 1 0,0 0 0));[Pose(Point Z(0 0 0),1,0,0,0)@2026-01-01, Pose(Point Z(4 0 0),0,0,0,1)@2026-01-02]'));
-- A sequence set covers what each of its sequences sweeps and nothing in the
-- gaps between them, in 2D, with Z, and where a sequence holds one instant
SELECT ST_AsText(round(traversedArea(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));{[Pose(Point(0 0),0)@2026-01-01, Pose(Point(1 0),0)@2026-01-02], [Pose(Point(5 0),0)@2026-01-03, Pose(Point(6 0),0)@2026-01-04]}', TRUE), 6));
SELECT ST_AsText(round(traversedArea(
  trgeometry 'Polygon Z((0 0 0,1 0 0,1 1 0,0 1 0,0 0 0));{[Pose(Point Z(0 0 0),1,0,0,0)@2026-01-01, Pose(Point Z(1 0 0),1,0,0,0)@2026-01-02], [Pose(Point Z(5 0 0),1,0,0,0)@2026-01-03, Pose(Point Z(6 0 0),1,0,0,0)@2026-01-04]}', TRUE), 6));
SELECT ST_AsText(round(traversedArea(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));{[Pose(Point(0 0),0)@2026-01-01, Pose(Point(1 0),0)@2026-01-02], [Pose(Point(5 0),0)@2026-01-03]}', TRUE), 6));
SELECT ST_Covers(traversedArea(
  trgeometry 'Polygon((-2 -0.1,2 -0.1,2 0.1,-2 0.1,-2 -0.1));{[Pose(Point(0 0),0)@2026-01-01, Pose(Point(0 0),0.04)@2026-01-02], [Pose(Point(9 0),0)@2026-01-03, Pose(Point(9 0),0.04)@2026-01-04]}', TRUE),
  geometry 'Point(5 0)');
