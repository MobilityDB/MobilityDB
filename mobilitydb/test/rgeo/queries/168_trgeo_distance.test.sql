-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------


-------------------------------------------------------------------------------
-- Temporal distance between a moving rigid geometry and a geometry, a
-- spatiotemporal box and another rigid geometry.
--
-- The distance of a fixed closest-feature pair curves while the body rotates,
-- so the temporal distance captures the interior turning point: for a rotating
-- body the nearest approach is strictly below the distance at both segment
-- ends.
-------------------------------------------------------------------------------

-- A 2 x 1 rectangle translating past a fixed square: the nearest approach is
-- realized halfway along the motion, below the distance at both ends
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Polygon((5 3,6 3,6 4,5 4,5 3))')::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Polygon((5 3,6 3,6 4,5 4,5 3))'))::numeric, 6);

-- A long rectangle rotating half a turn in place near a fixed square: the
-- nearest approach happens mid-rotation, below the distance at both ends
SELECT round(startValue(tDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  geometry 'Polygon((0 3,1 3,1 4,0 4,0 3))'))::numeric, 6);
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  geometry 'Polygon((0 3,1 3,1 4,0 4,0 3))')::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  geometry 'Polygon((0 3,1 3,1 4,0 4,0 3))'))::numeric, 6);

-- A rectangle rotating half a turn about one of its corners sweeps over a fixed
-- point and over a fixed polygon: the nearest approach is zero, realized over
-- the sub-interval of the rotation during which the body covers the target
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,3 0,3 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  geometry 'Point(2 2)')::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,3 0,3 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  geometry 'Point(2 2)'))::numeric, 6);
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,3 0,3 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  geometry 'Polygon((1.5 1.5,2.5 1.5,2.5 2.5,1.5 2.5,1.5 1.5))')::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,3 0,3 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  geometry 'Polygon((1.5 1.5,2.5 1.5,2.5 2.5,1.5 2.5,1.5 1.5))'))::numeric, 6);

-- Two convex polygons whose closest features are a facing pair of edges: the
-- distance handles the edge-edge closest-feature state and reports their
-- separation
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((2 2,2 1,4 0,4 4,2 2));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),0)@2001-01-02]',
  geometry 'Polygon((5 0,2 1,1 1,5 0))')::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((2 2,2 1,4 0,4 4,2 2));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),0)@2001-01-02]',
  geometry 'Polygon((5 0,2 1,1 1,5 0))'))::numeric, 6);

-- The distance operator agrees with the function
SELECT round(minValue(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]' <->
  geometry 'Polygon((5 3,6 3,6 4,5 4,5 3))')::numeric, 6);

-- Nearest approach distance to a spatiotemporal box
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  stbox 'STBOX X((20,20),(21,21))')::numeric, 6);

-- The shortest line at the nearest approach has length equal to that distance
SELECT round(ST_Length(shortestLine(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Polygon((5 3,6 3,6 4,5 4,5 3))'))::numeric, 6);

-- An instant rigid geometry: the distance is measured from the body placed by
-- the pose of the instant, and it is answered for every geometry type because
-- the instant case delegates to the geometry-geometry distance
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));Pose(Point(20 10),1.1)@2001-01-01',
  geometry 'Point(5 3)'))::numeric, 6);
SELECT round(ST_Distance(valueAtTimestamp(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));Pose(Point(20 10),1.1)@2001-01-01',
  '2001-01-01'), geometry 'Point(5 3)')::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));Pose(Point(20 10),1.1)@2001-01-01',
  geometry 'Linestring(5 3,6 4)'))::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));Pose(Point(20 10),1.1)@2001-01-01',
  geometry 'Polygon((5 3,6 3,6 4,5 4,5 3))'))::numeric, 6);
SELECT round(minValue(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));Pose(Point(20 10),1.1)@2001-01-01' <->
  geometry 'Polygon((5 3,6 3,6 4,5 4,5 3))')::numeric, 6);
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));Pose(Point(20 10),1.1)@2001-01-01',
  geometry 'Polygon((5 3,6 3,6 4,5 4,5 3))')::numeric, 6);

-- The nearest approach instant of the rotating body lies strictly inside the
-- motion, not at an endpoint
SELECT getTimestamp(nearestApproachInstant(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  geometry 'Polygon((0 3,1 3,1 4,0 4,0 3))')) <@ tstzspan '(2001-01-01, 2001-01-02)';

-- The temporal distance of a moving rigid geometry is linear, as it is in every
-- other spatial family: it has a value at every instant of the motion, not only
-- at the closest-feature transitions and the interior extrema
SELECT interp(tDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Point(0 5)'));
SELECT round(valueAtTimestamp(tDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Point(0 5)'), '2001-01-01 12:00')::numeric, 6);
SELECT interp(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Polygon((5 3,6 3,6 4,5 4,5 3))'));

-- The shortest line joins the body and the geometry themselves, not the two
-- geometries read as if they were points
SELECT ST_AsText(shortestLine(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Polygon((5 3,6 3,6 4,5 4,5 3))'));
SELECT ST_AsText(shortestLine(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Point(5 3)'));

-------------------------------------------------------------------------------
