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

-- bodyPointTrajectory(trgeometry, geometry) → tgeompoint:
-- world-frame trajectory of an arbitrary body-frame point on a moving rigid
-- geometry. The reference-point case (body point = origin) must agree with
-- the existing trgeometry::tgeompoint cast.

-------------------------------------------------------------------------------
-- 2D: a unit-square body translates from origin to (10,0) while yawing 90°.
-------------------------------------------------------------------------------

-- Body origin (0,0) trajectory must match the reference cast.
SELECT asText(round(bodyPointTrajectory(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),1.5707963267948966)@2026-01-01 00:01:00]',
  'Point(0 0)'), 6));

SELECT asText((trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),1.5707963267948966)@2026-01-01 00:01:00]')::tgeompoint);

-- Body front-right corner (1,0): at t1 (yaw=0) world (1,0); at t2 (yaw=π/2)
-- the body's +X axis now points +Y, so the corner is at world (10,1).
SELECT asText(round(bodyPointTrajectory(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),1.5707963267948966)@2026-01-01 00:01:00]',
  'Point(1 0)'), 6));

-- Body back-left corner (0,1): at t1 world (0,1); at t2 world (9,0) (the
-- corner now extends in the body's -Y direction, which is -X in the world).
SELECT asText(round(bodyPointTrajectory(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),1.5707963267948966)@2026-01-01 00:01:00]',
  'Point(0 1)'), 6));

-------------------------------------------------------------------------------
-- 2D static body (no motion): trajectory is constant.
-------------------------------------------------------------------------------

SELECT asText(round(bodyPointTrajectory(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(5 5),0)@2026-01-01, Pose(Point(5 5),0)@2026-01-01 00:01:00]',
  'Point(2 3)'), 6));

-------------------------------------------------------------------------------
-- Errors
-------------------------------------------------------------------------------

/* Errors */

-- Body argument must be a point.
SELECT bodyPointTrajectory(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),0)@2026-01-01 00:01:00]',
  'Linestring(0 0, 1 1)');

-- Body-point dimensionality must match the trgeometry's pose.
SELECT bodyPointTrajectory(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),0)@2026-01-01 00:01:00]',
  'PointZ(0 0 1)');

-------------------------------------------------------------------------------
