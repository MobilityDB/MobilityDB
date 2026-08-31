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
-- Temporal distance between two moving temporal rigid geometries.
--
-- Both reference bodies move, so the distance of a fixed closest-feature pair
-- curves as they rotate; the temporal distance captures the interior turning
-- point where the nearest approach happens, and it is symmetric in its two
-- arguments.
-------------------------------------------------------------------------------

-- Two unit squares approaching head on: the nearest approach is realized when
-- the moving one has slid closest to the other
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(20 0),0)@2001-01-01, Pose(Point(14 0),0)@2001-01-02]')::numeric, 6);

-- A long rectangle rotating half a turn next to a body that stays put: the
-- nearest approach happens mid-rotation, below the distance at both ends
SELECT round(startValue(tDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 3),0)@2001-01-01, Pose(Point(0 3),0)@2001-01-02]'))::numeric, 6);
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 3),0)@2001-01-01, Pose(Point(0 3),0)@2001-01-02]')::numeric, 6);
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 3),0)@2001-01-01, Pose(Point(0 3),0)@2001-01-02]'))::numeric, 6);

-- Both bodies rotating and translating independently
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(3 1),1.2)@2001-01-02]',
  trgeometry 'Polygon((0 0,3 0,3 1,0 1,0 0));[Pose(Point(9 0),0)@2001-01-01, Pose(Point(5 2),-0.8)@2001-01-02]')::numeric, 6);

-- The temporal distance is symmetric in its two arguments
SELECT round(minValue(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(3 1),1.2)@2001-01-02]',
  trgeometry 'Polygon((0 0,3 0,3 1,0 1,0 0));[Pose(Point(9 0),0)@2001-01-01, Pose(Point(5 2),-0.8)@2001-01-02]') -
  tDistance(
  trgeometry 'Polygon((0 0,3 0,3 1,0 1,0 0));[Pose(Point(9 0),0)@2001-01-01, Pose(Point(5 2),-0.8)@2001-01-02]',
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(3 1),1.2)@2001-01-02]'))::numeric, 6);

-- The distance operator agrees with the function
SELECT round(minValue(
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]' <->
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(20 0),0)@2001-01-01, Pose(Point(14 0),0)@2001-01-02]')::numeric, 6);

-- The nearest approach instant of the rotating pair lies strictly inside
-- the motion
SELECT getTimestamp(nearestApproachInstant(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 3),0)@2001-01-01, Pose(Point(0 3),0)@2001-01-02]')) <@ tstzspan '(2001-01-01, 2001-01-02)';
-- The temporal distance between two moving rigid geometries is linear
SELECT interp(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(20 0),0)@2001-01-01, Pose(Point(14 0),0)@2001-01-02]'));

-- The shortest line joins the two bodies themselves
SELECT ST_AsText(shortestLine(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(20 0),0)@2001-01-01, Pose(Point(14 0),0)@2001-01-02]'));

-- Two long hulls passing 2.2 m apart on nearly parallel headings: the
-- closest-feature walk reaches a transition a few 1e-14 short of the end of a
-- segment, where the root bracket of the next solver is already narrower than
-- the tolerance. The values are given as binary because the walk only reaches
-- that state for these exact doubles: the same coordinates written as text,
-- even at fifteen decimals, take a different path and answer without ever
-- entering it.
SELECT round((
  trgeometryFromHexEWKB('013E004EE86400000103000020E8640000010000000600000000000000008042C000000000000024C0000000000000E03F00000000000024C00000000000002A4000000000000004C0000000000000E03F000000000000144000000000008042C0000000000000144000000000008042C000000000000024C0020000000341E8640000CB68450025791C41E4DD9A25F3705741A815A15DDEECDBBFC00A61459AB5020041E8640000EBC5FCE324791C4168ACBB24F3705741A815A15DDEECDBBF80E308469AB50200') |=|
  trgeometryFromHexEWKB('013E004EE86400000103000020E864000001000000060000000000000000002EC000000000000010C00000000000E0404000000000000010C00000000000004940000000000000F03F0000000000E0404000000000000018400000000000002EC000000000000018400000000000002EC000000000000010C0020000000341E864000051052C3FD2781C41C34A561BF17057419EAFD4151EC80540C00A61459AB5020041E864000065BAFDF6D0781C41663FE1F7F0705741620A90885FA4054080E308469AB50200'))::numeric, 9);

-------------------------------------------------------------------------------
