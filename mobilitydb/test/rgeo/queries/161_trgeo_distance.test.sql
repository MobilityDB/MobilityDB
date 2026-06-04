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
-- Temporal distance between a temporal rigid geometry and a temporal point
-------------------------------------------------------------------------------

-- Temporal distance of a moving box against a moving point
SELECT round(tdistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(10 0),3.141592653589793)@2000-01-02]',
  tgeompoint '[Point(3 8)@2000-01-01, Point(12 8)@2000-01-02]'), 6);

-- Commutative form (tgeompoint, trgeometry)
SELECT round(tdistance(
  tgeompoint '[Point(3 8)@2000-01-01, Point(12 8)@2000-01-02]',
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(10 0),3.141592653589793)@2000-01-02]'), 6);

-- Distance operator <->
SELECT round(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(10 0),3.141592653589793)@2000-01-02]' <->
  tgeompoint '[Point(3 8)@2000-01-01, Point(12 8)@2000-01-02]', 6);

-- Instantaneous temporal rigid geometry and point
SELECT round(tdistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));Pose(Point(0 0),0)@2000-01-01',
  tgeompoint 'Point(5 0)@2000-01-01'), 6);

-- Degenerate static point: must match tdistance(trgeometry, geometry)
SELECT round(tdistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(10 0),3.141592653589793)@2000-01-02]',
  tgeompoint '[Point(5 0)@2000-01-01, Point(5 0)@2000-01-02]'), 6) =
  round(tdistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(10 0),3.141592653589793)@2000-01-02]',
  geometry 'Point(5 0)'), 6);

-------------------------------------------------------------------------------
-- Nearest approach distance
-------------------------------------------------------------------------------

SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(10 0),3.141592653589793)@2000-01-02]',
  tgeompoint '[Point(3 8)@2000-01-01, Point(12 8)@2000-01-02]')::numeric, 6);

SELECT round(nearestApproachDistance(
  tgeompoint '[Point(3 8)@2000-01-01, Point(12 8)@2000-01-02]',
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(10 0),3.141592653589793)@2000-01-02]')::numeric, 6);

SELECT round((
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(10 0),3.141592653589793)@2000-01-02]' |=|
  tgeompoint '[Point(3 8)@2000-01-01, Point(12 8)@2000-01-02]')::numeric, 6);

-------------------------------------------------------------------------------
-- Nearest approach instant
-------------------------------------------------------------------------------

SELECT asText(nearestApproachInstant(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(10 0),3.141592653589793)@2000-01-02]',
  tgeompoint '[Point(3 8)@2000-01-01, Point(12 8)@2000-01-02]'), 6);

SELECT asText(nearestApproachInstant(
  tgeompoint '[Point(3 8)@2000-01-01, Point(12 8)@2000-01-02]',
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(10 0),3.141592653589793)@2000-01-02]'), 6);

-------------------------------------------------------------------------------
