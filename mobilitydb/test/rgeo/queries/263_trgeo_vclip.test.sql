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

-- V-Clip tests.  All functions return the distance between the two convex
-- objects at the specified instant (or between static objects).

-------------------------------------------------------------------------------
-- v_clip_poly_point — static polygon vs static point
-------------------------------------------------------------------------------

-- Point outside -> positive distance
SELECT round(v_clip_poly_point(
  geometry 'Polygon((0 0,1 0,1 1,0 1,0 0))',
  geometry 'Point(5 0)'), 6);

-- Point at (4,0): closest polygon vertex is (1,0), distance = 3
SELECT round(v_clip_poly_point(
  geometry 'Polygon((0 0,1 0,1 1,0 1,0 0))',
  geometry 'Point(4 0)'), 6);

-------------------------------------------------------------------------------
-- v_clip_poly_poly — static polygon vs static polygon
-------------------------------------------------------------------------------

-- Touching polygons -> 0
SELECT v_clip_poly_poly(
  geometry 'Polygon((0 0,1 0,1 1,0 1,0 0))',
  geometry 'Polygon((1 0,2 0,2 1,1 1,1 0))');

-- Separated polygons -> positive distance
SELECT round(v_clip_poly_poly(
  geometry 'Polygon((0 0,1 0,1 1,0 1,0 0))',
  geometry 'Polygon((5 0,6 0,6 1,5 1,5 0))'), 6);

-------------------------------------------------------------------------------
-- v_clip_tpoly_point — moving polygon vs static point at a given instant
-------------------------------------------------------------------------------

-- At t=2001-01-05, body has translated to [4,0]-[5,1].
-- Point(0.5 0.5) is now far away -> positive distance
SELECT round(v_clip_tpoly_point(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]',
  geometry 'Point(0.5 0.5)',
  timestamptz '2001-01-05'), 6);

-- Instant outside temporal range -> NULL
SELECT v_clip_tpoly_point(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]',
  geometry 'Point(0.5 0.5)',
  timestamptz '2000-01-01');

-------------------------------------------------------------------------------
-- v_clip_tpoly_poly — moving polygon vs static polygon at a given instant
-------------------------------------------------------------------------------

-- At t=2001-01-01 bodies touch at x=1 -> 0
SELECT v_clip_tpoly_poly(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]',
  geometry 'Polygon((1 0,2 0,2 1,1 1,1 0))',
  timestamptz '2001-01-01');

-- At t=2001-01-05 bodies coincide -> 0
SELECT v_clip_tpoly_poly(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]',
  geometry 'Polygon((4 0,5 0,5 1,4 1,4 0))',
  timestamptz '2001-01-05');

-- At t=2001-01-01 bodies are separated by 4 units
SELECT round(v_clip_tpoly_poly(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]',
  geometry 'Polygon((5 0,6 0,6 1,5 1,5 0))',
  timestamptz '2001-01-01'), 6);

-------------------------------------------------------------------------------
-- v_clip_tpoly_tpoint — moving polygon vs moving point at a given instant
-------------------------------------------------------------------------------

-- At t=2001-01-01 polygon contains the point -> 0
SELECT v_clip_tpoly_tpoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]',
  tgeompoint '[Point(0.5 0.5)@2001-01-01, Point(0.5 0.5)@2001-01-05]',
  timestamptz '2001-01-01');

-- At t=2001-01-05 polygon has moved to [4,0]-[5,1], point still at (0.5,0.5) -> distance = 3
SELECT round(v_clip_tpoly_tpoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]',
  tgeompoint '[Point(0.5 0.5)@2001-01-01, Point(0.5 0.5)@2001-01-05]',
  timestamptz '2001-01-05'), 6);

-------------------------------------------------------------------------------
-- v_clip_tpoly_tpoly — two moving polygons at a given instant
-------------------------------------------------------------------------------

-- Same polygon at the same position -> 0
SELECT v_clip_tpoly_tpoly(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]',
  timestamptz '2001-01-03');

-- Two polygons separated by 4 units at t=2001-01-01
SELECT round(v_clip_tpoly_tpoly(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(4 0),0.0)@2001-01-05]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 0),0.0)@2001-01-01',
  timestamptz '2001-01-01'), 6);

-------------------------------------------------------------------------------
