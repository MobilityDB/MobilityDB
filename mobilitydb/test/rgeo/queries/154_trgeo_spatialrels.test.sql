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
-- eContains / aContains
-------------------------------------------------------------------------------

-- Big square contains the full traversal of the unit square moving along x
SELECT eContains(
  geometry 'Polygon((-1 -1,5 -1,5 2,-1 2,-1 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT aContains(
  geometry 'Polygon((-1 -1,5 -1,5 2,-1 2,-1 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

-- trgeo contains a small geometry point
SELECT eContains(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Point(2 0.5)');

SELECT aContains(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Point(2 0.5)');

-- trgeo × trgeo: each traverses the same area
SELECT eContains(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT aContains(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

-------------------------------------------------------------------------------
-- eCovers / aCovers
-------------------------------------------------------------------------------

SELECT eCovers(
  geometry 'Polygon((-1 -1,5 -1,5 2,-1 2,-1 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT aCovers(
  geometry 'Polygon((-1 -1,5 -1,5 2,-1 2,-1 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT eCovers(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Point(2 0.5)');

SELECT aCovers(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Point(2 0.5)');

SELECT eCovers(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT aCovers(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

-- A pose covers what lies within it, including a point of its boundary, and
-- covers no region larger than itself
SELECT eCovers(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),0)@2001-01-05]',
  geometry 'Point(0 0)');
SELECT aCovers(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),0)@2001-01-05]',
  geometry 'Point(0 0)');
SELECT eCovers(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),0)@2001-01-05]',
  geometry 'Polygon((-1 -1,-1 1,1 1,1 -1,-1 -1))');
-- A body sliding along the x axis covers a point on its path at one pose and
-- not at every one
SELECT eCovers(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Point(4.5 0.5)');
SELECT aCovers(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Point(4.5 0.5)');

-------------------------------------------------------------------------------
-- eDisjoint / aDisjoint
-------------------------------------------------------------------------------

SELECT eDisjoint(
  geometry 'Polygon((10 10,11 10,11 11,10 11,10 10))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT aDisjoint(
  geometry 'Polygon((10 10,11 10,11 11,10 11,10 10))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT eDisjoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Polygon((10 10,11 10,11 11,10 11,10 10))');

SELECT aDisjoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Polygon((10 10,11 10,11 11,10 11,10 10))');

SELECT eDisjoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(10 10),0)@2001-01-01');

SELECT aDisjoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(10 10),0)@2001-01-01');

-- A body standing on a line meets it at every pose, and one that starts away
-- from a region is disjoint from it at some pose
SELECT eDisjoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),0)@2001-01-05]',
  geometry 'Linestring(0 -5,0 5)');
SELECT eDisjoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(-5 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-05]',
  geometry 'Polygon((-1 -1,-1 1,1 1,1 -1,-1 -1))');

-------------------------------------------------------------------------------
-- eIntersects / aIntersects
-------------------------------------------------------------------------------

SELECT eIntersects(
  geometry 'Polygon((-1 -1,2 -1,2 2,-1 2,-1 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT aIntersects(
  geometry 'Polygon((-1 -1,2 -1,2 2,-1 2,-1 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT eIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Polygon((-1 -1,2 -1,2 2,-1 2,-1 -1))');

SELECT aIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Polygon((-1 -1,2 -1,2 2,-1 2,-1 -1))');

SELECT eIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT aIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

-- A body that never reaches the geometry does not always intersect it, and one
-- that stays within the geometry throughout does
SELECT eIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(20 20),0)@2001-01-01, Pose(Point(30 30),0)@2001-01-05]',
  geometry 'Polygon((-1 -1,2 -1,2 2,-1 2,-1 -1))');
SELECT aIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(20 20),0)@2001-01-01, Pose(Point(30 30),0)@2001-01-05]',
  geometry 'Polygon((-1 -1,2 -1,2 2,-1 2,-1 -1))');
SELECT aIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(1 0),0)@2001-01-05]',
  geometry 'Polygon((-1 -1,4 -1,4 4,-1 4,-1 -1))');
SELECT aIntersects(
  geometry 'Polygon((-1 -1,4 -1,4 4,-1 4,-1 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(1 0),0)@2001-01-05]');
-- A body that overlaps the region at every pose without ever being contained
-- in it meets it always
SELECT eIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Polygon((0.5 -1,20 -1,20 2,0.5 2,0.5 -1))');
SELECT aIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Polygon((0.5 -1,20 -1,20 2,0.5 2,0.5 -1))');

-------------------------------------------------------------------------------
-- eTouches / aTouches
-------------------------------------------------------------------------------

-- traversal boundary touches the geometry
SELECT eTouches(
  geometry 'Polygon((5 -1,6 -1,6 2,5 2,5 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT aTouches(
  geometry 'Polygon((5 -1,6 -1,6 2,5 2,5 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT eTouches(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Polygon((5 -1,6 -1,6 2,5 2,5 -1))');

SELECT aTouches(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Polygon((5 -1,6 -1,6 2,5 2,5 -1))');

SELECT eTouches(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(1 0),0)@2001-01-01');

SELECT aTouches(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(1 0),0)@2001-01-01');

-------------------------------------------------------------------------------
-- eDwithin / aDwithin
-------------------------------------------------------------------------------

SELECT eDwithin(
  geometry 'Point(6 0)',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  2.0);

SELECT aDwithin(
  geometry 'Point(6 0)',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  2.0);

SELECT eDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Point(6 0)',
  2.0);

SELECT aDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Point(6 0)',
  2.0);

SELECT eDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(3 0),0)@2001-01-01',
  3.0);

SELECT aDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(3 0),0)@2001-01-01',
  3.0);

-------------------------------------------------------------------------------
