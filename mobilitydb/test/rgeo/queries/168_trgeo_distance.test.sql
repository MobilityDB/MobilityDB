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

-- A box with a period measures the part of the rigid geometry inside it
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  stbox 'STBOX XT(((20,20),(21,21)),[2001-01-01, 2001-01-01 12:00])')::numeric, 6);
-- No part of the rigid geometry is inside the period
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  stbox 'STBOX XT(((20,20),(21,21)),[2001-02-01, 2001-02-02])')::numeric, 6);

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

-- A rigid geometry given as a sequence set: the reference geometry is stored
-- once by the sequence set and not by its composing sequences, so it is read
-- from the temporal value and passed down to every sequence
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));{[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02],[Pose(Point(0 0),0)@2001-01-03, Pose(Point(10 0),0)@2001-01-04]}',
  geometry 'Point(5 3)'))::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));{[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02],[Pose(Point(0 0),0)@2001-01-03, Pose(Point(10 0),0)@2001-01-04]}',
  geometry 'Polygon((5 3,6 3,6 4,5 4,5 3))'))::numeric, 6);
SELECT numSequences(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));{[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02],[Pose(Point(0 0),0)@2001-01-03, Pose(Point(10 0),0)@2001-01-04]}',
  geometry 'Point(5 3)'));
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));{[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.14159265)@2001-01-02],[Pose(Point(0 0),0)@2001-01-03, Pose(Point(5 0),0)@2001-01-04]}',
  geometry 'Polygon((0 3,1 3,1 4,0 4,0 3))')::numeric, 6);

-- A multi-component target: the distance is the minimum over the components, so
-- the closest component switches while the body translates and the temporal
-- distance kinks where the two component distances cross
SELECT round(tDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'MultiPoint(0 5,11 5)'), 6);
SELECT round(tDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'MultiPolygon(((0 5,1 5,1 6,0 6,0 5)),((11 5,12 5,12 6,11 6,11 5)))'), 6);

-- A single-component multi geometry answers exactly as the component alone does
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'MultiPolygon(((5 3,6 3,6 4,5 4,5 3)))'))::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'MultiPoint(5 3)'))::numeric, 6);

-- An empty component contributes no point to be close to and is ignored, as it
-- is by the geometry-geometry distance
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'MultiPolygon(((5 3,6 3,6 4,5 4,5 3)),EMPTY)'))::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'MultiPoint(5 3,EMPTY)'))::numeric, 6);

-- A rotating body against a multi-component target, on a sequence set
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));{[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.14159265)@2001-01-02],[Pose(Point(0 0),0)@2001-01-03, Pose(Point(5 0),0)@2001-01-04]}',
  geometry 'MultiPolygon(((0 3,1 3,1 4,0 4,0 3)),((8 0,9 0,9 1,8 1,8 0)))'))::numeric, 6);

-- The nearest approach distance and the shortest line agree with the component
-- that realizes the minimum
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'MultiPolygon(((5 3,6 3,6 4,5 4,5 3)),((100 100,101 100,101 101,100 101,100 100)))')::numeric, 6);

-- A line target: the distance to a line is the minimum over its segments, each
-- of which is a convex set whose two vertices share one edge
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Linestring(5 3,6 4)'))::numeric, 6);
-- The closest point of the line is interior to a segment, not an endpoint
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Linestring(-5 3,20 3)'))::numeric, 6);
-- A line of several segments answers as the nearest of its segments does
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Linestring(-5 9,4 9,4 3,20 3)'))::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Linestring(4 3,20 3)'))::numeric, 6);
-- A rotating body against a line: the nearest approach lies strictly inside
-- the motion
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.14159265)@2001-01-02]',
  geometry 'Linestring(0 3,1 4)')::numeric, 6);
SELECT getTimestamp(nearestApproachInstant(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.14159265)@2001-01-02]',
  geometry 'Linestring(0 3,1 4)')) <@ tstzspan '(2001-01-01, 2001-01-02)';
-- A multi-component line, and an empty component that is ignored
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'MultiLinestring((5 3,6 4),(0 -9,10 -9))'))::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'MultiLinestring((5 3,6 4),EMPTY)'))::numeric, 6);
-- The shortest line to a line joins the body and the line themselves
SELECT ST_AsText(ST_SnapToGrid(shortestLine(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Linestring(-5 3,20 3)'), 0.000001));

-- Every turning point of a folded distance carries the distance at that
-- instant. Where two components cross, neither carries a turning point of its
-- own, so a minimum taken over their interpolations would sit above the value
-- the geometry-geometry distance answers there
SELECT round(abs(valueAtTimestamp(tDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'MultiPoint(0 5,11 5)'), '2001-01-01 12:00')
  - ST_Distance(valueAtTimestamp(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  '2001-01-01 12:00'), geometry 'MultiPoint(0 5,11 5)'))::numeric, 9);
SELECT round(abs(valueAtTimestamp(tDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'MultiLinestring((0 5,0 6),(11 5,11 6))'), '2001-01-01 12:00')
  - ST_Distance(valueAtTimestamp(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  '2001-01-01 12:00'), geometry 'MultiLinestring((0 5,0 6),(11 5,11 6))'))::numeric, 9);

-------------------------------------------------------------------------------
-- The distance of a rigid geometry to a geometry stands at their separation
-- while they are apart and at zero once they meet, so the join between the
-- two is a kink and the moment of contact is a point of the answer. The rod
-- below covers y in [c-0.2, c+0.2] at centre height c and travels from c = 0
-- to c = 6, so it meets the band y in [4,5] exactly while c lies in
-- [3.8, 5.2], that is from 15:12 to 20:48 of the first day.
-------------------------------------------------------------------------------

SELECT round(valueAtTimestamp(tDistance(
  trgeometry 'Polygon((-3 -0.2,3 -0.2,3 0.2,-3 0.2,-3 -0.2));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 6),0)@2001-01-02]',
  geometry 'Polygon((-0.5 4,0.5 4,0.5 5,-0.5 5,-0.5 4))'),
  '2001-01-01 15:11:00')::numeric, 6);

SELECT round(valueAtTimestamp(tDistance(
  trgeometry 'Polygon((-3 -0.2,3 -0.2,3 0.2,-3 0.2,-3 -0.2));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 6),0)@2001-01-02]',
  geometry 'Polygon((-0.5 4,0.5 4,0.5 5,-0.5 5,-0.5 4))'),
  '2001-01-01 15:30:00')::numeric, 6);

-- A point target and a linestring target reach the same floor at the same time.

SELECT round(valueAtTimestamp(tDistance(
  trgeometry 'Polygon((-3 -0.2,3 -0.2,3 0.2,-3 0.2,-3 -0.2));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 6),0)@2001-01-02]',
  geometry 'Point(0 4)'), '2001-01-01 15:30:00')::numeric, 6);

SELECT round(valueAtTimestamp(tDistance(
  trgeometry 'Polygon((-3 -0.2,3 -0.2,3 0.2,-3 0.2,-3 -0.2));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 6),0)@2001-01-02]',
  geometry 'Linestring(-1 4,1 4)'), '2001-01-01 15:30:00')::numeric, 6);

-- Where the closest features change at the moment of contact the walk already
-- answers it exactly, and a body that only grazes reaches the floor at a
-- single instant. A square covering y in [c-1, c+1] travelling from c = 0 to
-- c = 1 touches the band y in [2,3] at the last instant alone.

SELECT round(valueAtTimestamp(tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 3),0)@2001-01-02]',
  geometry 'Polygon((-1 2,1 2,1 3,-1 3,-1 2))'),
  '2001-01-01 08:00:00')::numeric, 6);

SELECT round(valueAtTimestamp(tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 1),0)@2001-01-02]',
  geometry 'Polygon((-1 2,1 2,1 3,-1 3,-1 2))'),
  '2001-01-01 23:59:00')::numeric, 6);

-- At each instant of a moving body the distance is the one of the body placed
-- there. The triangle at the pose (1 0) is at distance 1 from (0 3), at the
-- pose (0.5 0.25) at distance 0.5, and at the pose (0 0) it has (0 3) on its
-- edge.

SELECT tDistance(
  trgeometry 'Polygon((0 0,6 0,0 6,0 0));[Pose(Point(1 0),0)@2001-01-01 00:00:10, Pose(Point(0 0),0)@2001-01-01 00:00:35, Pose(Point(0.5 0.25),0)@2001-01-01 00:00:45, Pose(Point(2 1),0)@2001-01-01 00:01:35]',
  geometry 'Point(0 3)');

-- A body at rest touching a point is at distance 0 all along

SELECT tDistance(
  trgeometry 'Polygon((0 0,6 0,0 6,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),0)@2001-01-02]',
  geometry 'Point(0 3)');

-- A body that slides along a point: the 4 x 4 square has (3 4) on its top
-- edge while it moves left, and in it afterwards, so the distance is 0 all
-- along

SELECT tDistance(
  trgeometry 'Polygon((0 0,4 0,4 4,0 4,0 0));[Pose(Point(3 0),0)@2001-01-01 00:00:05, Pose(Point(-1 0),0)@2001-01-01 00:00:20, Pose(Point(0 0),0)@2001-01-01 00:01:10, Pose(Point(2 1),0)@2001-01-01 00:01:25]',
  geometry 'Point(3 4)');

-- The same with an edge that is not along an axis: (-6 6) is the end of the
-- top edge of the body, which moves left along it

SELECT tDistance(
  trgeometry 'Polygon((-4 4,2 -4,3 -3,3 0,1 4,-4 4));[Pose(Point(-2 2),0)@2001-01-01, Pose(Point(-4.5 2),0)@2001-01-02]',
  geometry 'Point(-6 6)');

-- A vertex on the line between its neighbors is not a corner of the body:
-- (1 2) is on the top edge of the square, which is convex. A vertex 1e-13
-- below that line is a corner turning inward, and a body that is not convex
-- is refused

SELECT tDistance(
  trgeometry 'Polygon((0 0,2 0,2 2,1 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),0)@2001-01-02]',
  geometry 'Point(1 5)');
SELECT tDistance(
  trgeometry 'Polygon((0 0,2 0,2 2,1 1.9999999999999,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),0)@2001-01-02]',
  geometry 'Point(1 5)');

-- A body that does not turn is nearest a geometry inside a segment: its
-- vertex (3 -1) moves from (5 0) to (0 2) while the triangle goes from the
-- pose (2 1) to (-3 3), and passes (4 1) at the distance 3 / sqrt(29), the
-- double 0.55708601453115558 nearest it

SELECT nearestApproachDistance(
  trgeometry 'Polygon((-3 -2,-2 -5,3 -1,-3 -2));[Pose(Point(2 1),0)@2001-01-01 00:00:00, Pose(Point(-3 3),0)@2001-01-01 00:06:05.630507, Pose(Point(-3 -3),0)@2001-01-01 00:13:58.721992, Pose(Point(-3 -2),0)@2001-01-01 00:55:53.96598]',
  geometry 'Point(4 1)') = 0.55708601453115558;

-- At an instant, a body that does not turn is its reference geometry moved
-- exactly by the position of the pose: the vertex (2.623 2.152) at the pose
-- (1.869 -2.829) is at x = 4.492, the sum of the two doubles, which rounds to
-- a double 2.2e-16 below it. The distance of the exact body to the line is
-- the double 0.043970339265039356 nearest it, where the body with the
-- rounded sums is at the distance 0.04397033926503958

SELECT nearestApproachDistance(trg, geo) = 0.043970339265039356,
  valueAtTimestamp(tDistance(trg, geo), '2001-01-01 00:21:01.796393') =
    0.043970339265039356
FROM (SELECT
  trgeometry 'Polygon((-4.876 2.124,-2.334 -2.237,-1.084 -1.315,2.623 2.152,-0.252 4.765,-4.876 2.124));[Pose(Point(-0.384 0.392),0)@2001-01-01 00:00:00, Pose(Point(1.869 -2.829),0)@2001-01-01 00:21:01.796393]' AS trg,
  geometry 'LineString(4.498 -2.698,4.619 3.741)' AS geo) t;

-------------------------------------------------------------------------------
