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
-- tContains
-------------------------------------------------------------------------------

-- tContains(geometry, trgeometry): body inside geometry -> true
SELECT tContains(
  geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5), 0.0)@2001-01-01');

-- tContains(geometry, trgeometry): body outside geometry -> false
SELECT tContains(
  geometry 'Polygon((0 0,1 1,1 0,0 0))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5), 0.0)@2001-01-01');

-- tContains(trgeometry, geometry): body contains a small polygon -> true
SELECT tContains(
  trgeometry 'Polygon((0 0,10 0,10 10,0 10,0 0));Pose(Point(0 0), 0.0)@2001-01-01',
  geometry 'Polygon((1 1,2 1,2 2,1 2,1 1))');

-- tContains(trgeometry, geometry): body does not contain far polygon -> false
SELECT tContains(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0), 0.0)@2001-01-01',
  geometry 'Polygon((5 5,6 5,6 6,5 6,5 5))');

-- tContains(trgeometry, trgeometry): body1 contains body2 -> true
SELECT tContains(
  trgeometry 'Polygon((0 0,10 0,10 10,0 10,0 0));Pose(Point(0 0), 0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5), 0.0)@2001-01-01');

-------------------------------------------------------------------------------
-- tCovers
-------------------------------------------------------------------------------

-- tCovers(geometry, trgeometry): body inside geometry -> true
SELECT tCovers(
  geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5), 0.0)@2001-01-01');

-- tCovers(trgeometry, geometry): body covers a small polygon -> true
SELECT tCovers(
  trgeometry 'Polygon((0 0,10 0,10 10,0 10,0 0));Pose(Point(0 0), 0.0)@2001-01-01',
  geometry 'Polygon((1 1,2 1,2 2,1 2,1 1))');

-- tCovers(trgeometry, trgeometry): body1 covers body2 -> true
SELECT tCovers(
  trgeometry 'Polygon((0 0,10 0,10 10,0 10,0 0));Pose(Point(0 0), 0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5), 0.0)@2001-01-01');

-------------------------------------------------------------------------------
-- tIntersects
-------------------------------------------------------------------------------

-- tIntersects(trgeometry, geometry): overlapping -> true
SELECT tIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5), 0.0)@2001-01-01',
  geometry 'Polygon((4 4,6 4,6 6,4 6,4 4))');

-- tIntersects(trgeometry, geometry): disjoint -> false
SELECT tIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5), 0.0)@2001-01-01',
  geometry 'Polygon((10 10,11 10,11 11,10 11,10 10))');

-- tIntersects(geometry, trgeometry): overlapping -> true
SELECT tIntersects(
  geometry 'Polygon((4 4,6 4,6 6,4 6,4 4))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5), 0.0)@2001-01-01');

-- tIntersects(trgeometry, trgeometry): same body -> true
SELECT tIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0), 0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0), 0.0)@2001-01-01');

-- tIntersects sequence
SELECT tIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01,
              Pose(Point(2 0), 0.0)@2001-01-05]',
  geometry 'Polygon((1 -1,3 -1,3 2,1 2,1 -1))');

-------------------------------------------------------------------------------
-- tDisjoint
-------------------------------------------------------------------------------

-- tDisjoint(trgeometry, geometry): disjoint -> true
SELECT tDisjoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5), 0.0)@2001-01-01',
  geometry 'Polygon((0 0,1 0,1 0,0 0))');

-- tDisjoint(geometry, trgeometry): overlapping -> false
SELECT tDisjoint(
  geometry 'Polygon((4 4,6 4,6 6,4 6,4 4))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5), 0.0)@2001-01-01');

-- tDisjoint(trgeometry, trgeometry): bodies far apart -> true
SELECT tDisjoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0), 0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5), 0.0)@2001-01-01');

-------------------------------------------------------------------------------
-- tTouches
-------------------------------------------------------------------------------

-- tTouches(trgeometry, geometry): sharing a boundary edge -> true
SELECT tTouches(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0), 0.0)@2001-01-01',
  geometry 'Polygon((1 0,2 0,2 1,1 1,1 0))');

-- tTouches(trgeometry, geometry): overlapping -> false
SELECT tTouches(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0), 0.0)@2001-01-01',
  geometry 'Polygon((4 4,6 4,6 6,4 6,4 4))');

-- tTouches(geometry, trgeometry): sharing a boundary edge -> true
SELECT tTouches(
  geometry 'Polygon((1 0,2 0,2 1,1 1,1 0))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0), 0.0)@2001-01-01');

-- tTouches(trgeometry, trgeometry): adjacent bodies -> true
SELECT tTouches(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0), 0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(1 0), 0.0)@2001-01-01');

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

-- tDwithin(trgeometry, geometry, dist): within distance -> true
SELECT tDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5), 0.0)@2001-01-01',
  geometry 'Polygon((4 4,6 4,6 6,4 6,4 4))',
  1.0);

-- tDwithin(trgeometry, geometry, dist): beyond distance -> false
SELECT tDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5), 0.0)@2001-01-01',
  geometry 'Polygon((10 10,11 10,11 11,10 11,10 10))',
  1.0);

-- tDwithin(geometry, trgeometry, dist): within distance -> true
SELECT tDwithin(
  geometry 'Polygon((4 4,6 4,6 6,4 6,4 4))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5), 0.0)@2001-01-01',
  1.0);

-- tDwithin(trgeometry, trgeometry, dist): bodies close -> true
SELECT tDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0), 0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(2 0), 0.0)@2001-01-01',
  1.5);

-- tDwithin(trgeometry, trgeometry, dist): bodies far -> false
SELECT tDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0), 0.0)@2001-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(10 10), 0.0)@2001-01-01',
  1.0);

-------------------------------------------------------------------------------
