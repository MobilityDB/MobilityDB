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
-- Space boxes
-------------------------------------------------------------------------------

SELECT array_length(spaceBoxes(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1), 0.0)@2000-01-01, Pose(Point(10 10), 0.0)@2000-01-10]',
  2.0), 1) > 0;

SELECT array_length(spaceBoxes(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1), 0.0)@2000-01-01, Pose(Point(10 10), 0.0)@2000-01-10]',
  2.0, 2.0), 1) > 0;

SELECT array_length(spaceBoxes(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1), 0.0)@2000-01-01, Pose(Point(10 10), 0.0)@2000-01-10]',
  2.0, 2.0, 2.0), 1) > 0;

-------------------------------------------------------------------------------
-- Time boxes
-------------------------------------------------------------------------------

SELECT array_length(timeBoxes(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1), 0.0)@2000-01-01, Pose(Point(10 10), 0.0)@2000-01-10]',
  interval '3 days', '2000-01-01'), 1) > 0;

-------------------------------------------------------------------------------
-- SpaceTime boxes
-------------------------------------------------------------------------------

SELECT array_length(spaceTimeBoxes(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1), 0.0)@2000-01-01, Pose(Point(10 10), 0.0)@2000-01-10]',
  2.0, interval '3 days'), 1) > 0;

SELECT array_length(spaceTimeBoxes(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1), 0.0)@2000-01-01, Pose(Point(10 10), 0.0)@2000-01-10]',
  2.0, 2.0, interval '3 days'), 1) > 0;

SELECT array_length(spaceTimeBoxes(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1), 0.0)@2000-01-01, Pose(Point(10 10), 0.0)@2000-01-10]',
  2.0, 2.0, 2.0, interval '3 days'), 1) > 0;

-------------------------------------------------------------------------------
-- Space split
-------------------------------------------------------------------------------

SELECT COUNT(*) > 0 FROM spaceSplit(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1), 0.0)@2000-01-01, Pose(Point(10 10), 0.0)@2000-01-10]',
  2.0);

SELECT COUNT(*) > 0 FROM spaceSplit(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1), 0.0)@2000-01-01, Pose(Point(10 10), 0.0)@2000-01-10]',
  2.0, 2.0);

SELECT COUNT(*) > 0 FROM spaceSplit(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1), 0.0)@2000-01-01, Pose(Point(10 10), 0.0)@2000-01-10]',
  2.0, 2.0, 2.0);

-------------------------------------------------------------------------------
-- SpaceTime split
-------------------------------------------------------------------------------

SELECT COUNT(*) > 0 FROM spaceTimeSplit(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1), 0.0)@2000-01-01, Pose(Point(10 10), 0.0)@2000-01-10]',
  2.0, interval '3 days');

SELECT COUNT(*) > 0 FROM spaceTimeSplit(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1), 0.0)@2000-01-01, Pose(Point(10 10), 0.0)@2000-01-10]',
  2.0, 2.0, interval '3 days');

SELECT COUNT(*) > 0 FROM spaceTimeSplit(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1), 0.0)@2000-01-01, Pose(Point(10 10), 0.0)@2000-01-10]',
  2.0, 2.0, 2.0, interval '3 days');

-------------------------------------------------------------------------------
