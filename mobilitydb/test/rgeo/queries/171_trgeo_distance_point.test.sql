-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
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
