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

-- Helper: a simple moving square
-- Pose(Point(0 0),0)@t1 => Pose(Point(4 0),0)@t5: translates right 4 units over 5 days

-------------------------------------------------------------------------------
-- tdistance — instant and discrete
-------------------------------------------------------------------------------

-- TInstant: identity pose → standard Euclidean distance from world polygon
SELECT round(tdistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2000-01-01',
  geometry 'Point(2 2)'), 6);
-- TInstant: non-identity pose → polygon shifts to (5,5)-(6,5)-(6,6)-(5,6)
SELECT round(tdistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5),0)@2000-01-01',
  geometry 'Point(2 2)'), 6);
-- Discrete sequence: per-instant Euclidean distance with pose applied
SELECT round(tdistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));{Pose(Point(0 0),0)@2000-01-01, Pose(Point(5 5),0)@2000-01-02}',
  geometry 'Point(2 2)'), 6);

-------------------------------------------------------------------------------
-- tdistance — continuous sequence
-------------------------------------------------------------------------------

SELECT round(tdistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]',
  geometry 'Point(2 2)'), 6);
SELECT round(tdistance(
  geometry 'Point(2 2)',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]'), 6);
SELECT round(tdistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]',
  tgeompoint '[Point(2 2)@2000-01-01, Point(2 2)@2000-01-05]'), 6);
SELECT round(tdistance(
  tgeompoint '[Point(2 2)@2000-01-01, Point(2 2)@2000-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]'), 6);
SELECT round(tdistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 5),0)@2000-01-01, Pose(Point(4 5),0)@2000-01-05]'), 6);

-------------------------------------------------------------------------------
-- nearestApproachInstant
-------------------------------------------------------------------------------

SELECT asText(nearestApproachInstant(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2000-01-01',
  geometry 'Point(2 2)'));
SELECT asText(nearestApproachInstant(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]',
  geometry 'Point(2 2)'));
SELECT asText(nearestApproachInstant(
  geometry 'Point(2 2)',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]'));
SELECT asText(nearestApproachInstant(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]',
  tgeompoint '[Point(2 2)@2000-01-01, Point(2 2)@2000-01-05]'));
SELECT asText(nearestApproachInstant(
  tgeompoint '[Point(2 2)@2000-01-01, Point(2 2)@2000-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]'));
SELECT asText(nearestApproachInstant(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 5),0)@2000-01-01, Pose(Point(4 5),0)@2000-01-05]'));

-------------------------------------------------------------------------------
-- nearestApproachDistance
-------------------------------------------------------------------------------

SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2000-01-01',
  geometry 'Point(2 2)'), 6);
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]',
  geometry 'Point(2 2)'), 6);
SELECT round(nearestApproachDistance(
  geometry 'Point(2 2)',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]'), 6);
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]',
  stbox 'STBOX X((1,-1),(3,3))'), 6);
SELECT round(nearestApproachDistance(
  stbox 'STBOX X((1,-1),(3,3))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]'), 6);
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]',
  tgeompoint '[Point(2 2)@2000-01-01, Point(2 2)@2000-01-05]'), 6);
SELECT round(nearestApproachDistance(
  tgeompoint '[Point(2 2)@2000-01-01, Point(2 2)@2000-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]'), 6);
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 5),0)@2000-01-01, Pose(Point(4 5),0)@2000-01-05]'), 6);

-------------------------------------------------------------------------------
-- shortestLine
-------------------------------------------------------------------------------

SELECT ST_AsText(shortestLine(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2000-01-01',
  geometry 'Point(2 2)'));
SELECT ST_AsText(shortestLine(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]',
  geometry 'Point(2 2)'));
SELECT ST_AsText(shortestLine(
  geometry 'Point(2 2)',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]'));
SELECT ST_AsText(shortestLine(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]',
  tgeompoint '[Point(2 2)@2000-01-01, Point(2 2)@2000-01-05]'));
SELECT ST_AsText(shortestLine(
  tgeompoint '[Point(2 2)@2000-01-01, Point(2 2)@2000-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]'));
SELECT ST_AsText(shortestLine(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(4 0),0)@2000-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 5),0)@2000-01-01, Pose(Point(4 5),0)@2000-01-05]'));

-------------------------------------------------------------------------------
-- Table queries
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  tdistance(temp, ST_SetSRID('Point(50 50)'::geometry, 5676)) IS NOT NULL;

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  nearestApproachInstant(temp, ST_SetSRID('Point(50 50)'::geometry, 5676)) IS NOT NULL;

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  nearestApproachDistance(temp, ST_SetSRID('Point(50 50)'::geometry, 5676)) IS NOT NULL;

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  shortestLine(temp, ST_SetSRID('Point(50 50)'::geometry, 5676)) IS NOT NULL;

-------------------------------------------------------------------------------
