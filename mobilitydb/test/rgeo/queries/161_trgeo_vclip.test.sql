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
-- v_clip_poly_point
-------------------------------------------------------------------------------

-- Point inside polygon
SELECT round(v_clip_poly_point(
  'Polygon((0 0,2 0,2 2,0 2,0 0))'::geometry,
  'Point(1 1)'::geometry), 6);
-- Point outside polygon
SELECT round(v_clip_poly_point(
  'Polygon((0 0,2 0,2 2,0 2,0 0))'::geometry,
  'Point(3 3)'::geometry), 6);
-- Point on boundary
SELECT round(v_clip_poly_point(
  'Polygon((0 0,2 0,2 2,0 2,0 0))'::geometry,
  'Point(1 0)'::geometry), 6);

-------------------------------------------------------------------------------
-- v_clip_poly_poly
-------------------------------------------------------------------------------

-- Overlapping polygons
SELECT round(v_clip_poly_poly(
  'Polygon((0 0,2 0,2 2,0 2,0 0))'::geometry,
  'Polygon((1 1,3 1,3 3,1 3,1 1))'::geometry), 6);
-- Disjoint polygons
SELECT round(v_clip_poly_poly(
  'Polygon((0 0,1 0,1 1,0 1,0 0))'::geometry,
  'Polygon((2 2,3 2,3 3,2 3,2 2))'::geometry), 6);
-- One inside the other
SELECT round(v_clip_poly_poly(
  'Polygon((0 0,4 0,4 4,0 4,0 0))'::geometry,
  'Polygon((1 1,3 1,3 3,1 3,1 1))'::geometry), 6);

-------------------------------------------------------------------------------
-- v_clip_tpoly_point (trgeometry × static point at given timestamptz)
-------------------------------------------------------------------------------

SELECT round(v_clip_tpoly_point(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2000-01-01',
  'Point(0.5 0.5)'::geometry,
  timestamptz '2000-01-01'), 6);
SELECT round(v_clip_tpoly_point(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5),0)@2000-01-01',
  'Point(0.5 0.5)'::geometry,
  timestamptz '2000-01-01'), 6);

-------------------------------------------------------------------------------
-- v_clip_tpoly_poly (trgeometry × static polygon at given timestamptz)
-------------------------------------------------------------------------------

SELECT round(v_clip_tpoly_poly(
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));Pose(Point(0 0),0)@2000-01-01',
  'Polygon((1 1,3 1,3 3,1 3,1 1))'::geometry,
  timestamptz '2000-01-01'), 6);
SELECT round(v_clip_tpoly_poly(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5),0)@2000-01-01',
  'Polygon((1 1,3 1,3 3,1 3,1 1))'::geometry,
  timestamptz '2000-01-01'), 6);

-------------------------------------------------------------------------------
-- v_clip_tpoly_tpoint (trgeometry × tgeompoint at given timestamptz)
-------------------------------------------------------------------------------

SELECT round(v_clip_tpoly_tpoint(
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));Pose(Point(0 0),0)@2000-01-01',
  tgeompoint 'Point(1 1)@2000-01-01',
  timestamptz '2000-01-01'), 6);
SELECT round(v_clip_tpoly_tpoint(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5),0)@2000-01-01',
  tgeompoint 'Point(1 1)@2000-01-01',
  timestamptz '2000-01-01'), 6);

-------------------------------------------------------------------------------
-- v_clip_tpoly_tpoly (trgeometry × trgeometry at given timestamptz)
-------------------------------------------------------------------------------

SELECT round(v_clip_tpoly_tpoly(
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));Pose(Point(0 0),0)@2000-01-01',
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));Pose(Point(1 1),0)@2000-01-01',
  timestamptz '2000-01-01'), 6);
SELECT round(v_clip_tpoly_tpoly(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2000-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(5 5),0)@2000-01-01',
  timestamptz '2000-01-01'), 6);

-------------------------------------------------------------------------------
