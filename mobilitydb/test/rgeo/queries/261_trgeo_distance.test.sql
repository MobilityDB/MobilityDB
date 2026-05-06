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
-- Body polygon at Pose(Point(0 0),0.0) at t1 moving to Pose(Point(4 0),0.0)
-- at t5.  Reference object Point(5 0) lies to the right: distance starts at 4
-- and reaches 0 when the polygon's rightmost edge aligns with the reference.

-------------------------------------------------------------------------------
-- tdistance — trgeo <-> geometry (sequence only)
-------------------------------------------------------------------------------

SELECT asText(round(tdistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]',
  geometry 'Point(5 0)'), 6));

SELECT asText(round(tdistance(
  geometry 'Point(5 0)',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]'),
  6));

SELECT asText(round(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]'
  <-> geometry 'Point(5 0)', 6));

-------------------------------------------------------------------------------
-- nearestApproachInstant
-------------------------------------------------------------------------------

SELECT asText(nearestApproachInstant(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]',
  geometry 'Point(5 0)'));

SELECT asText(nearestApproachInstant(
  geometry 'Point(5 0)',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]'));

-------------------------------------------------------------------------------
-- nearestApproachDistance
-------------------------------------------------------------------------------

SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]',
  geometry 'Point(5 0)'), 6);

SELECT round(nearestApproachDistance(
  geometry 'Point(5 0)',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]'),
  6);

SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]',
  stbox 'STBOX X((5, 0), (6, 1))'), 6);

SELECT round(nearestApproachDistance(
  stbox 'STBOX X((5, 0), (6, 1))',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]'),
  6);

SELECT round(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]'
  |=| geometry 'Point(5 0)', 6);

-------------------------------------------------------------------------------
