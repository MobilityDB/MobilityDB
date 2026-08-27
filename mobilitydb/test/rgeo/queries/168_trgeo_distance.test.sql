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
