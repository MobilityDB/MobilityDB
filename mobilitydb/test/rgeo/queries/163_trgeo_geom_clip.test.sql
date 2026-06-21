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
-------------------------------------------------------------------------------

-- Pure-translation trapezoid-polygon clip primitive (M1)

-- Test 1: moving segment fully outside polygon -> NULL
SELECT _trgeometry_geom_clip_polygon(
  geometry 'Point(0 0)', geometry 'Point(1 0)',
  geometry 'Point(0 5)', geometry 'Point(1 5)',
  geometry 'Polygon((10 10, 20 10, 20 20, 10 20, 10 10))');

-- Test 2: moving segment fully inside polygon -> {[0, 1]}
SELECT _trgeometry_geom_clip_polygon(
  geometry 'Point(1 1)', geometry 'Point(2 1)',
  geometry 'Point(1 2)', geometry 'Point(2 2)',
  geometry 'Polygon((0 0, 10 0, 10 10, 0 10, 0 0))');

-- Test 3: moving segment enters polygon at t=0.5 -> {[0.5, 1]}
SELECT _trgeometry_geom_clip_polygon(
  geometry 'Point(0 0)', geometry 'Point(1 0)',
  geometry 'Point(0 4)', geometry 'Point(1 4)',
  geometry 'Polygon((0 2, 5 2, 5 5, 0 5, 0 2))');

-- Test 4: moving segment straddles polygon (in between t=0.3 and t=0.5) -> {[0.3, 0.5]}
SELECT _trgeometry_geom_clip_polygon(
  geometry 'Point(0 0)', geometry 'Point(1 0)',
  geometry 'Point(0 10)', geometry 'Point(1 10)',
  geometry 'Polygon((0 3, 5 3, 5 5, 0 5, 0 3))');

-- Test 5: pure-translation precondition violated -> error
SELECT _trgeometry_geom_clip_polygon(
  geometry 'Point(0 0)', geometry 'Point(1 0)',
  geometry 'Point(0 1)', geometry 'Point(2 1)',
  geometry 'Polygon((0 0, 10 0, 10 10, 0 10, 0 0))');

-- Test 6: degenerate polygon -> error
SELECT _trgeometry_geom_clip_polygon(
  geometry 'Point(0 0)', geometry 'Point(1 0)',
  geometry 'Point(0 1)', geometry 'Point(1 1)',
  geometry 'LINESTRING(0 0, 1 0)');

-- Test 7: zero displacement (static segment) -> {[0, 1]} if intersects, NULL otherwise
SELECT _trgeometry_geom_clip_polygon(
  geometry 'Point(1 1)', geometry 'Point(2 1)',
  geometry 'Point(1 1)', geometry 'Point(2 1)',
  geometry 'Polygon((0 0, 10 0, 10 10, 0 10, 0 0))');

-- Posed entry — translation only (theta1 == theta2): should match the
-- translation-only fast path.
SELECT _trgeometry_geom_clip_polygon_posed(
  geometry 'Point(0 0)', geometry 'Point(1 0)',
  pose 'Pose(Point(0 0), 0)', pose 'Pose(Point(0 4), 0)',
  geometry 'Polygon((0 2, 5 2, 5 5, 0 5, 0 2))');

-- Posed entry — small rotation (Taylor closed-form path), 5 degrees
SELECT _trgeometry_geom_clip_polygon_posed(
  geometry 'Point(0 0)', geometry 'Point(1 0)',
  pose 'Pose(Point(0 0), 0)', pose 'Pose(Point(0 4), 0.0872665)',
  geometry 'Polygon((0 2, 5 2, 5 5, 0 5, 0 2))') IS NOT NULL;

-- Posed entry — large rotation (numerical solver), 60 degrees
SELECT _trgeometry_geom_clip_polygon_posed(
  geometry 'Point(0 0)', geometry 'Point(1 0)',
  pose 'Pose(Point(0 0), 0)', pose 'Pose(Point(0 4), 1.0472)',
  geometry 'Polygon((0 2, 5 2, 5 5, 0 5, 0 2))') IS NOT NULL;

-------------------------------------------------------------------------------
