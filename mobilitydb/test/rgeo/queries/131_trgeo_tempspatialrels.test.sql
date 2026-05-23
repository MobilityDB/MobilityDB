-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
-- tContains
-------------------------------------------------------------------------------

SELECT tContains(
  geometry 'Polygon((-1 -1,5 -1,5 2,-1 2,-1 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT tContains(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Point(2 0.5)');

SELECT tContains(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

-------------------------------------------------------------------------------
-- tCovers
-------------------------------------------------------------------------------

SELECT tCovers(
  geometry 'Polygon((-1 -1,5 -1,5 2,-1 2,-1 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT tCovers(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Point(2 0.5)');

SELECT tCovers(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

-------------------------------------------------------------------------------
-- tDisjoint
-------------------------------------------------------------------------------

SELECT tDisjoint(
  geometry 'Point(100 100)',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT tDisjoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Point(100 100)');

SELECT tDisjoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  trgeometry 'Polygon((10 10,11 10,11 11,10 11,10 10));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

-------------------------------------------------------------------------------
-- tIntersects
-------------------------------------------------------------------------------

SELECT tIntersects(
  geometry 'Polygon((-1 -1,5 -1,5 2,-1 2,-1 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT tIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Point(2 0.5)');

SELECT tIntersects(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

-------------------------------------------------------------------------------
-- tTouches
-------------------------------------------------------------------------------

SELECT tTouches(
  geometry 'Polygon((-1 -1,0 -1,0 0,-1 0,-1 -1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

SELECT tTouches(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Point(0 0)');

SELECT tTouches(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  trgeometry 'Polygon((1 0,2 0,2 1,1 1,1 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]');

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

SELECT tDwithin(
  geometry 'Point(0 0)',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  0.5);

SELECT tDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  geometry 'Point(2 0.5)',
  0.5);

SELECT tDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  1.0);

-------------------------------------------------------------------------------
