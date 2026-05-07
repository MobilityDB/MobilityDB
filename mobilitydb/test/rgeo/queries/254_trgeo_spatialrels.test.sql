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
-- eIntersects, aIntersects
-- A unit square body at the origin; traversed area = the body polygon.
-------------------------------------------------------------------------------

-- eIntersects(geometry, trgeometry): overlapping geometry -> true
SELECT eIntersects(
  geometry 'Polygon((0.5 0.5,2 0.5,2 2,0.5 2,0.5 0.5))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01');
-- eIntersects(geometry, trgeometry): disjoint geometry -> false
SELECT eIntersects(
  geometry 'Polygon((5 5,6 5,6 6,5 6,5 5))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01');

-- eIntersects(trgeometry, geometry): overlapping geometry -> true
SELECT eIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  geometry 'Polygon((0.5 0.5,2 0.5,2 2,0.5 2,0.5 0.5))');
-- eIntersects(trgeometry, geometry): disjoint geometry -> false
SELECT eIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  geometry 'Polygon((5 5,6 5,6 6,5 6,5 5))');

-- eIntersects(trgeometry, trgeometry): two squares at same spot -> true
SELECT eIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01');
-- eIntersects(trgeometry, trgeometry): two disjoint squares -> false
SELECT eIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5),0.0)@2001-01-01');

-- aIntersects(geometry, trgeometry)
SELECT aIntersects(
  geometry 'Polygon((-1 -1,2 -1,2 2,-1 2,-1 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01');
-- aIntersects(trgeometry, geometry)
SELECT aIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  geometry 'Polygon((-1 -1,2 -1,2 2,-1 2,-1 -1))');
-- aIntersects(trgeometry, trgeometry)
SELECT aIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01');

-- eIntersects over a sequence (traversed area covers both positions)
SELECT eIntersects(
  geometry 'Polygon((1 -0.5,2 -0.5,2 1.5,1 1.5,1 -0.5))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(2 0),0.0)@2001-01-05]');

-------------------------------------------------------------------------------
-- eDisjoint, aDisjoint
-------------------------------------------------------------------------------

-- eDisjoint(geometry, trgeometry): disjoint -> true
SELECT eDisjoint(
  geometry 'Polygon((5 5,6 5,6 6,5 6,5 5))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01');
-- eDisjoint(geometry, trgeometry): overlapping -> false
SELECT eDisjoint(
  geometry 'Polygon((0.5 0.5,2 0.5,2 2,0.5 2,0.5 0.5))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01');

-- eDisjoint(trgeometry, geometry): disjoint -> true
SELECT eDisjoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  geometry 'Polygon((5 5,6 5,6 6,5 6,5 5))');
-- eDisjoint(trgeometry, geometry): overlapping -> false
SELECT eDisjoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  geometry 'Polygon((0.5 0.5,2 0.5,2 2,0.5 2,0.5 0.5))');

-- eDisjoint(trgeometry, trgeometry): disjoint -> true
SELECT eDisjoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5),0.0)@2001-01-01');
-- eDisjoint(trgeometry, trgeometry): overlapping -> false
SELECT eDisjoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01');

-------------------------------------------------------------------------------
-- eContains, aContains
-------------------------------------------------------------------------------

-- eContains(trgeometry, geometry): traversed area contains a small point -> true
SELECT eContains(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  geometry 'Point(0.5 0.5)');
-- eContains(trgeometry, geometry): traversed area does not contain far point -> false
SELECT eContains(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  geometry 'Point(5 5)');

-- eContains(geometry, trgeometry): large geometry contains the traversed area -> true
SELECT eContains(
  geometry 'Polygon((-1 -1,2 -1,2 2,-1 2,-1 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01');

-- eContains(trgeometry, trgeometry): one square contains itself -> true
SELECT eContains(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01');

-------------------------------------------------------------------------------
-- eCovers, aCovers
-------------------------------------------------------------------------------

-- eCovers(trgeometry, geometry)
SELECT eCovers(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  geometry 'Point(0.5 0.5)');
SELECT eCovers(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  geometry 'Point(5 5)');

-- eCovers(geometry, trgeometry)
SELECT eCovers(
  geometry 'Polygon((-1 -1,2 -1,2 2,-1 2,-1 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01');

-- eCovers(trgeometry, trgeometry)
SELECT eCovers(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01');

-------------------------------------------------------------------------------
-- eTouches, aTouches
-------------------------------------------------------------------------------

-- eTouches(trgeometry, geometry): sharing only a boundary edge -> true
SELECT eTouches(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  geometry 'Polygon((1 0,2 0,2 1,1 1,1 0))');
-- eTouches(trgeometry, geometry): overlapping interior -> false
SELECT eTouches(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  geometry 'Polygon((0.5 0,2 0,2 1,0.5 1,0.5 0))');

-- eTouches(geometry, trgeometry)
SELECT eTouches(
  geometry 'Polygon((1 0,2 0,2 1,1 1,1 0))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01');

-- eTouches(trgeometry, trgeometry)
SELECT eTouches(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(1 0),0.0)@2001-01-01');

-------------------------------------------------------------------------------
-- eDwithin, aDwithin
-------------------------------------------------------------------------------

-- eDwithin(trgeometry, geometry, dist): within distance -> true
SELECT eDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  geometry 'Polygon((2 0,3 0,3 1,2 1,2 0))',
  1.5);
-- eDwithin(trgeometry, geometry, dist): beyond distance -> false
SELECT eDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  geometry 'Polygon((5 0,6 0,6 1,5 1,5 0))',
  1.5);

-- eDwithin(geometry, trgeometry, dist)
SELECT eDwithin(
  geometry 'Polygon((2 0,3 0,3 1,2 1,2 0))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  1.5);

-- eDwithin(trgeometry, trgeometry, dist)
SELECT eDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(2 0),0.0)@2001-01-01',
  1.5);

-------------------------------------------------------------------------------
