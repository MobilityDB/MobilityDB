#!/usr/bin/env python3
#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2026, PostGIS contributors
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without a written
# agreement is hereby granted, provided that the above copyright notice and
# this paragraph and the following two paragraphs appear in all copies.
#
# IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
# LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
# EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.
#
# UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
# AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
#

"""The ledger of the plan of the temporal rigid geometries, doc/contributing/trgeometry.

  trgeo_gaps.py sql ledger      every function taking a trgeometry, over each body, motion and argument
  trgeo_gaps.py sql known       the cases whose answer the geometry fixes
  trgeo_gaps.py sql controls    their 2D controls, and the forms of traversedArea
  trgeo_gaps.py report OUTPUT   the calls that fail where the same call answers with one property of
                                the failing argument changed

A script runs in the test harness in the place of a test of the family, and OUTPUT is its harvested
output.
"""
import collections, re, sys

LEDGER = r'''SET client_min_messages = notice;
SET timezone = 'UTC';

CREATE TABLE gap_body(body text, wkt text, dim int);
INSERT INTO gap_body VALUES
  ('convex',  'Polygon((0 0,2 0,2 2,0 2,0 0))', 2),
  ('concave', 'Polygon((0 0,4 0,4 4,2 1,0 4,0 0))', 2),
  ('holes',   'Polygon((0 0,4 0,4 4,0 4,0 0),(1 1,2 1,2 2,1 2,1 1))', 2),
  ('surface', 'PolyhedralSurface(((0 0,2 0,2 2,0 2,0 0)),((2 0,4 0,4 2,2 2,2 0)))', 2),
  ('line',    'Linestring(0 0,2 2)', 2),
  ('point',   'Point(1 1)', 2),
  ('convex3d',  'Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1))', 3),
  ('concave3d', 'Polygon((0 0 1,4 0 1,4 4 1,2 1 1,0 4 1,0 0 1))', 3),
  ('surface3d', 'PolyhedralSurface(((0 0 0,2 0 0,2 2 0,0 0 0)),((0 0 0,2 2 0,0 0 2,0 0 0)))', 3);

CREATE TABLE gap_motion(motion text, dim int, lit text);
INSERT INTO gap_motion VALUES
  ('inst',      2, 'Pose(Point(0 0),0)@2001-01-01'),
  ('translate', 2, '[Pose(Point(0 0),0)@2001-01-01,Pose(Point(5 0),0)@2001-01-02]'),
  ('rotate',    2, '[Pose(Point(0 0),0)@2001-01-01,Pose(Point(5 0),1)@2001-01-02]'),
  ('discrete',  2, '{Pose(Point(0 0),0)@2001-01-01,Pose(Point(5 0),1)@2001-01-02}'),
  ('step',      2, 'Interp=Step;[Pose(Point(0 0),0)@2001-01-01,Pose(Point(5 0),1)@2001-01-02]'),
  ('inst',      3, 'Pose(Point(0 0 0),1,0,0,0)@2001-01-01'),
  ('translate', 3, '[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(5 0 1),1,0,0,0)@2001-01-02]'),
  ('rotate',    3, '[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(5 0 1),0.7071067811865476,0,0,0.7071067811865476)@2001-01-02]');

-- The values of each type a function is given
CREATE TABLE gap_sample(typ regtype, label text, expr text);
INSERT INTO gap_sample
SELECT 'trgeometry', b.body || '/' || m.motion,
  format('trgeometry %L', CASE WHEN m.motion = 'step'
    THEN 'SRID=5676;Interp=Step;' || b.wkt || ';' || replace(m.lit, 'Interp=Step;', '')
    ELSE 'SRID=5676;' || b.wkt || ';' || m.lit END)
FROM gap_body b, gap_motion m WHERE b.dim = m.dim;

INSERT INTO gap_sample VALUES
  ('geometry', 'g-point', $$geometry 'SRID=5676;Point(20 20)'$$),
  ('geometry', 'g-line', $$geometry 'SRID=5676;Linestring(20 20,30 30)'$$),
  ('geometry', 'g-convex', $$geometry 'SRID=5676;Polygon((20 20,22 20,22 22,20 22,20 20))'$$),
  ('geometry', 'g-concave', $$geometry 'SRID=5676;Polygon((20 20,24 20,24 24,22 21,20 24,20 20))'$$),
  ('geometry', 'g-holes', $$geometry 'SRID=5676;Polygon((20 20,24 20,24 24,20 24,20 20),(21 21,22 21,22 22,21 22,21 21))'$$),
  ('geometry', 'g-multipoly', $$geometry 'SRID=5676;MultiPolygon(((20 20,22 20,22 22,20 20)),((30 30,32 30,32 32,30 30)))'$$),
  ('geometry', 'g-collection', $$geometry 'SRID=5676;GeometryCollection(Point(20 20),Linestring(30 30,31 31))'$$),
  ('geometry', 'g-curve', $$geometry 'SRID=5676;CircularString(20 20,21 21,22 20)'$$),
  ('geometry', 'g-empty', $$geometry 'SRID=5676;Point empty'$$),
  ('geometry', 'g-point3d', $$geometry 'SRID=5676;Point(20 20 1)'$$),
  ('geometry', 'g-poly3d', $$geometry 'SRID=5676;Polygon((20 20 1,22 20 1,22 22 1,20 20 1))'$$),
  ('tgeompoint', 'tp-2d', $$tgeompoint 'SRID=5676;[Point(20 20)@2001-01-01,Point(30 30)@2001-01-02]'$$),
  ('tgeompoint', 'tp-3d', $$tgeompoint 'SRID=5676;[Point(20 20 1)@2001-01-01,Point(30 30 1)@2001-01-02]'$$),
  ('pose', 'pose-2d', $$pose 'SRID=5676;Pose(Point(1 1),0.5)'$$),
  ('pose', 'pose-3d', $$pose 'SRID=5676;Pose(Point(1 1 1),1,0,0,0)'$$),
  ('poseset', 'poseset-2d', $$poseset 'SRID=5676;{Pose(Point(1 1),0.5),Pose(Point(2 2),0.5)}'$$),
  ('tpose', 'tpose-2d', $$tpose 'SRID=5676;[Pose(Point(0 0),0)@2001-01-01,Pose(Point(5 0),1)@2001-01-02]'$$),
  ('stbox', 'b-x', $$stbox 'SRID=5676;STBOX X((0,0),(50,50))'$$),
  ('stbox', 'b-xt', $$stbox 'SRID=5676;STBOX XT(((0,0),(50,50)),[2001-01-01,2001-01-03])'$$),
  ('stbox', 'b-zt', $$stbox 'SRID=5676;STBOX ZT(((0,0,0),(50,50,50)),[2001-01-01,2001-01-03])'$$),
  ('stbox', 'b-t', $$stbox 'STBOX T([2001-01-01,2001-01-03])'$$),
  ('tstzspan', 'span', $$tstzspan '[2001-01-01,2001-01-02]'$$),
  ('tstzset', 'set', $$tstzset '{2001-01-01,2001-01-02}'$$),
  ('tstzspanset', 'spanset', $$tstzspanset '{[2001-01-01,2001-01-02]}'$$),
  ('timestamptz', 'ts', $$timestamptz '2001-01-01 12:00:00'$$),
  ('interval', 'iv', $$interval '1 hour'$$),
  ('float8', 'f', $$1.0::float8$$),
  ('int4', 'i', $$1::int4$$),
  ('int8', 'l', $$1::int8$$),
  ('int2', 's', $$1::int2$$),
  ('bool', 'bt', $$true$$),
  ('text', 'txt', $$'x'::text$$);

-- Report and drop the values that cannot be built, which is a finding by itself
DO $$
DECLARE r record; msg text; st text;
BEGIN
  FOR r IN SELECT * FROM gap_sample WHERE typ = 'trgeometry'::regtype LOOP
    BEGIN
      EXECUTE 'SELECT ' || r.expr;
      RAISE NOTICE 'BUILD|%|ok', r.label;
    EXCEPTION WHEN OTHERS THEN
      GET STACKED DIAGNOSTICS msg = MESSAGE_TEXT, st = RETURNED_SQLSTATE;
      RAISE NOTICE 'BUILD|%|%|%', r.label, st, msg;
      DELETE FROM gap_sample WHERE label = r.label;
    END;
  END LOOP;
END $$;

-- The second argument of a pair of bodies is one of the values whose motion is linear
CREATE TABLE gap_second AS
SELECT * FROM gap_sample
WHERE typ = 'trgeometry'::regtype AND (label LIKE '%/translate' OR label LIKE '%/rotate');

DO $$
DECLARE
  p record; a oid; i int; n int; ncall int := 0; nfn int := 0;
  combos text[][]; c text[]; lab text; q text; msg text; st text; d text;
  sel text; frm text; rec record; miss text;
BEGIN
  FOR p IN
    SELECT o.oid, o.proname, o.proargtypes::oid[] AS args,
      o.pronargs - o.pronargdefaults AS nreq,
      oidvectortypes(o.proargtypes) AS sig, prorettype::regtype::text AS ret
    FROM pg_proc o
    WHERE o.prokind = 'f' AND 'trgeometry'::regtype::oid = ANY(o.proargtypes::oid[])
      AND NOT EXISTS (SELECT 1 FROM unnest(o.proargtypes::oid[]) t(x)
        WHERE x IN ('internal'::regtype::oid, 'cstring'::regtype::oid, 'oid'::regtype::oid))
    ORDER BY 2, 5
  LOOP
    nfn := nfn + 1;
    n := p.nreq;
    -- A missing sample type is reported, never silently skipped
    SELECT string_agg(x::regtype::text, ',') INTO miss
    FROM unnest(p.args[0:n-1]) x
    WHERE NOT EXISTS (SELECT 1 FROM gap_sample s WHERE s.typ::oid = x);
    IF miss IS NOT NULL THEN
      RAISE NOTICE 'NOSAMPLE|%(%)|%', p.proname, p.sig, miss;
      CONTINUE;
    END IF;
    frm := '';
    sel := '';
    FOR i IN 0 .. n - 1 LOOP
      IF i > 0 THEN sel := sel || ', '; END IF;
      sel := sel || format('a%s.expr, a%s.label', i, i);
      frm := frm || CASE WHEN i > 0 THEN ', ' ELSE '' END ||
        CASE WHEN p.args[i] = 'trgeometry'::regtype::oid AND i > 0
          THEN format('gap_second a%s', i)
          ELSE format('gap_sample a%s', i) END;
    END LOOP;
    q := 'SELECT ARRAY[' || (SELECT string_agg(format('a%s.expr', j), ',') FROM generate_series(0, n - 1) j) ||
      '] AS exprs, ' || (SELECT string_agg(format('a%s.label', j), ' || '' '' || ') FROM generate_series(0, n - 1) j) ||
      ' AS lab FROM ' || frm || ' WHERE ' ||
      (SELECT string_agg(format('a%s.typ::oid = %s', j, p.args[j]), ' AND ') FROM generate_series(0, n - 1) j);
    FOR rec IN EXECUTE q LOOP
      ncall := ncall + 1;
      BEGIN
        EXECUTE format('SELECT %I(%s)', p.proname, array_to_string(rec.exprs, ', '));
        RAISE NOTICE 'OK|%(%)|%', p.proname, p.sig, rec.lab;
      EXCEPTION WHEN OTHERS THEN
        GET STACKED DIAGNOSTICS msg = MESSAGE_TEXT, st = RETURNED_SQLSTATE;
        RAISE NOTICE 'ERR|%(%)|%|%|%', p.proname, p.sig, rec.lab, st, msg;
      END;
    END LOOP;
  END LOOP;
  RAISE NOTICE 'DONE|functions %|calls %', nfn, ncall;
END $$;

SELECT prokind, COUNT(*) FROM pg_proc WHERE 'trgeometry'::regtype::oid = ANY(proargtypes::oid[]) GROUP BY 1 ORDER BY 1;
'''

KNOWN = r'''SET timezone = 'UTC';
SET client_min_messages = notice;
CREATE TABLE t(
  z1 trgeometry, zmove trgeometry, zrot trgeometry,
  xy trgeometry, xyrot trgeometry, cc trgeometry, ho trgeometry, su trgeometry, su3 trgeometry);
INSERT INTO t VALUES (
  -- At z = 1, moving along x
  'SRID=5676;Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(5 0 0),1,0,0,0)@2001-01-02]',
  -- Moving 3 along x and 4 along z, a length of 5
  'SRID=5676;Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(3 0 4),1,0,0,0)@2001-01-02]',
  -- Turning by pi/2 about z
  'SRID=5676;Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(0 0 0),0.7071067811865476,0,0,0.7071067811865476)@2001-01-02]',
  'SRID=5676;Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(5 0),0)@2001-01-02]',
  'SRID=5676;Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(0 0),1.5707963267948966)@2001-01-02]',
  -- A notch at (2,3), standing still
  'SRID=5676;Polygon((0 0,4 0,4 4,2 1,0 4,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(0 0),0)@2001-01-02]',
  -- A hole around (1.5,1.5), standing still
  'SRID=5676;Polygon((0 0,4 0,4 4,0 4,0 0),(1 1,2 1,2 2,1 2,1 1));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(0 0),0)@2001-01-02]',
  -- Two squares covering x in [0,4], y in [0,2], standing still
  'SRID=5676;PolyhedralSurface(((0 0,2 0,2 2,0 2,0 0)),((2 0,4 0,4 2,2 2,2 0)));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(0 0),0)@2001-01-02]',
  'SRID=5676;PolyhedralSurface(((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1)),((2 0 1,4 0 1,4 2 1,2 2 1,2 0 1)));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(0 0 0),1,0,0,0)@2001-01-02]');

CREATE TABLE q(name text, expected text, sql text);
INSERT INTO q VALUES
  -- 3D: a geometry over the same x, y at another z
  ('3d eIntersects z=1', 'true', $$eIntersects(z1, geometry 'SRID=5676;Point(1 1 1)')$$),
  ('3d eIntersects z=5', 'false', $$eIntersects(z1, geometry 'SRID=5676;Point(1 1 5)')$$),
  ('3d aIntersects z=5', 'false', $$aIntersects(z1, geometry 'SRID=5676;Point(1 1 5)')$$),
  ('3d eDisjoint z=5', 'true', $$eDisjoint(z1, geometry 'SRID=5676;Point(1 1 5)')$$),
  ('3d aDisjoint z=5', 'true', $$aDisjoint(z1, geometry 'SRID=5676;Point(1 1 5)')$$),
  ('3d tIntersects z=1', 'true then false', $$asText(tIntersects(z1, geometry 'SRID=5676;Point(1 1 1)'))$$),
  ('3d tIntersects z=5', 'all false', $$asText(tIntersects(z1, geometry 'SRID=5676;Point(1 1 5)'))$$),
  ('3d tDisjoint z=5', 'all true', $$asText(tDisjoint(z1, geometry 'SRID=5676;Point(1 1 5)'))$$),
  ('3d eContains z=5', 'false', $$eContains(geometry 'SRID=5676;Polygon((-9 -9 5,9 -9 5,9 9 5,-9 9 5,-9 -9 5))', z1)$$),
  ('3d eCovers z=5', 'false', $$eCovers(geometry 'SRID=5676;Polygon((-9 -9 5,9 -9 5,9 9 5,-9 9 5,-9 -9 5))', z1)$$),
  ('3d aContains', 'unknown', $$aContains(geometry 'SRID=5676;Polygon((-9 -9 5,9 -9 5,9 9 5,-9 9 5,-9 -9 5))', z1)$$),
  ('3d aCovers z=5', 'false', $$aCovers(geometry 'SRID=5676;Polygon((-9 -9 5,9 -9 5,9 9 5,-9 9 5,-9 -9 5))', z1)$$),
  ('3d eTouches z=5', 'false', $$eTouches(z1, geometry 'SRID=5676;Point(0 0 5)')$$),
  ('3d aTouches z=5', 'false', $$aTouches(z1, geometry 'SRID=5676;Point(0 0 5)')$$),
  -- 3D: two bodies at different z
  ('3d eIntersects bodies z=1,z=5', 'false', $$eIntersects(z1, (SELECT trgeometry 'SRID=5676;Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1));[Pose(Point(0 0 4),1,0,0,0)@2001-01-01,Pose(Point(5 0 4),1,0,0,0)@2001-01-02]'))$$),
  ('3d tIntersects bodies z=1,z=5', 'all false', $$asText(tIntersects(z1, (SELECT trgeometry 'SRID=5676;Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1));[Pose(Point(0 0 4),1,0,0,0)@2001-01-01,Pose(Point(5 0 4),1,0,0,0)@2001-01-02]')))$$),
  ('3d eDisjoint bodies z=1,z=5', 'true', $$eDisjoint(z1, (SELECT trgeometry 'SRID=5676;Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1));[Pose(Point(0 0 4),1,0,0,0)@2001-01-01,Pose(Point(5 0 4),1,0,0,0)@2001-01-02]'))$$),
  -- 3D: restrictions by a box at another z
  ('3d atStbox z in [4,6]', 'NULL', $$asText(atStbox(z1, stbox 'SRID=5676;STBOX ZT(((0,0,4),(9,9,6)),[2001-01-01,2001-01-03])'))$$),
  ('3d minusStbox z in [4,6]', 'whole value', $$asText(minusStbox(z1, stbox 'SRID=5676;STBOX ZT(((0,0,4),(9,9,6)),[2001-01-01,2001-01-03])'))$$),
  -- 3D: the measures of a motion by (3,0,4)
  ('3d length', '5', $$length(zmove)$$),
  ('3d cumulativeLength end', '5', $$endValue(cumulativeLength(zmove))$$),
  ('3d speed', '5/86400 = 0.0000578704', $$asText(speed(zmove))$$),
  ('3d value at end', 'z = 5', $$asEWKT(valueAtTimestamp(zmove, '2001-01-02'))$$),
  ('3d box', 'Z from 1 to 5', $$zmove::stbox::text$$),
  ('3d twcentroid', 'center z = 2', $$asEWKT(twCentroid(zmove))$$),
  ('3d centroid', 'center z', $$asText(centroid(zmove))$$),
  ('3d convexHull', 'spans z 1..5', $$asEWKT(convexHull(zmove))$$),
  ('3d traversedArea moving', 'a swept region', $$asEWKT(traversedArea(z1))$$),
  ('2d traversedArea moving', 'one rectangle, area 14', $$ST_Area(traversedArea(xy))::text || ' ' || ST_NumGeometries(traversedArea(xy))::text$$),
  ('3d traversedArea turning', 'a swept region', $$asEWKT(traversedArea(zrot))$$),
  ('3d value turned', 'x -2..0', $$asEWKT(valueAtTimestamp(zrot, '2001-01-02'))$$),
  ('3d angularSpeed', 'pi/2/86400 = 0.0000181805', $$asText(angularSpeed(zrot))$$),
  -- 2D: a point in the notch of a concave body, in the hole of a holed one
  ('concave eIntersects notch', 'false', $$eIntersects(cc, geometry 'SRID=5676;Point(2 3)')$$),
  ('concave eIntersects inside', 'true', $$eIntersects(cc, geometry 'SRID=5676;Point(2 0.5)')$$),
  ('concave eContains notch', 'false', $$eContains(cc, geometry 'SRID=5676;Point(2 3)')$$),
  ('concave tIntersects notch', 'all false', $$asText(tIntersects(cc, geometry 'SRID=5676;Point(2 3)'))$$),
  ('concave atGeometry notch', 'NULL', $$asText(atGeometry(cc, geometry 'SRID=5676;Polygon((1.9 2.9,2.1 2.9,2.1 3.1,1.9 3.1,1.9 2.9))'))$$),
  ('holes eIntersects hole', 'false', $$eIntersects(ho, geometry 'SRID=5676;Point(1.5 1.5)')$$),
  ('holes eIntersects ring', 'true', $$eIntersects(ho, geometry 'SRID=5676;Point(3 3)')$$),
  ('holes tIntersects hole', 'all false', $$asText(tIntersects(ho, geometry 'SRID=5676;Point(1.5 1.5)'))$$),
  ('holes atGeometry hole', 'NULL', $$asText(atGeometry(ho, geometry 'SRID=5676;Polygon((1.4 1.4,1.6 1.4,1.6 1.6,1.4 1.6,1.4 1.4))'))$$),
  ('holes traversedArea still', 'area 15', $$ST_Area(traversedArea(ho))::text$$),
  ('concave traversedArea still', 'area 10', $$ST_Area(traversedArea(cc))::text$$),
  -- 2D: a surface
  ('surface eIntersects in', 'true', $$eIntersects(su, geometry 'SRID=5676;Point(3 1)')$$),
  ('surface eIntersects out', 'false', $$eIntersects(su, geometry 'SRID=5676;Point(3 3)')$$),
  ('surface tIntersects in', 'all true', $$asText(tIntersects(su, geometry 'SRID=5676;Point(3 1)'))$$),
  ('surface atGeometry out', 'NULL', $$asText(atGeometry(su, geometry 'SRID=5676;Polygon((2.9 2.9,3.1 2.9,3.1 3.1,2.9 3.1,2.9 2.9))'))$$),
  ('surface traversedArea', 'area 8', $$ST_Area(traversedArea(su))::text$$),
  ('surface3d tIntersects', 'all true', $$asText(tIntersects(su3, geometry 'SRID=5676;Point(3 1 1)'))$$),
  ('surface3d eIntersects z=5', 'false', $$eIntersects(su3, geometry 'SRID=5676;Point(3 1 5)')$$);

DO $$
DECLARE r record; v text; msg text;
BEGIN
  FOR r IN SELECT * FROM q LOOP
    BEGIN
      EXECUTE 'SELECT (' || r.sql || ')::text FROM t' INTO v;
      RAISE NOTICE 'K|%|expected %|got %', r.name, r.expected, coalesce(left(v, 200), 'NULL');
    EXCEPTION WHEN OTHERS THEN
      GET STACKED DIAGNOSTICS msg = MESSAGE_TEXT;
      RAISE NOTICE 'K|%|expected %|ERROR %', r.name, r.expected, msg;
    END;
  END LOOP;
END $$;
'''

CONTROLS = r'''SET timezone = 'UTC';
SET client_min_messages = notice;
CREATE TABLE t(z1 trgeometry, xy trgeometry, zrot trgeometry, xyrot trgeometry, zmove trgeometry);
INSERT INTO t VALUES (
  'SRID=5676;Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(5 0 0),1,0,0,0)@2001-01-02]',
  'SRID=5676;Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(5 0),0)@2001-01-02]',
  'SRID=5676;Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(0 0 0),0.7071067811865476,0,0,0.7071067811865476)@2001-01-02]',
  'SRID=5676;Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(0 0),1.5707963267948966)@2001-01-02]',
  'SRID=5676;Polygon((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(3 0 4),1,0,0,0)@2001-01-02]');
CREATE TABLE q(name text, expected text, sql text);
INSERT INTO q VALUES
  ('2d traversedArea pieces', 'pieces', $$ST_NumGeometries(traversedArea(xy))::text || ' pieces, area ' || ST_Area(traversedArea(xy))::text$$),
  ('2d traversedArea merged', 'area 14', $$ST_Area(traversedArea(xy, true))::text$$),
  ('3d traversedArea merged', 'area 14 at z=1', $$ST_Area(traversedArea(z1, true))::text || ' ' || asEWKT(traversedArea(z1, true))$$),
  ('2d traversedArea turning merged', 'a swept region', $$ST_Area(traversedArea(xyrot, true))::text$$),
  ('3d traversedArea turning merged', 'as 2d', $$ST_Area(traversedArea(zrot, true))::text$$),
  ('3d traversedArea z move merged', 'unknown', $$asEWKT(traversedArea(zmove, true))$$),
  ('2d tIntersects', 'true then false', $$asText(tIntersects(xy, geometry 'SRID=5676;Point(1 1)'))$$),
  ('3d tIntersects same z', 'true then false', $$asText(tIntersects(z1, geometry 'SRID=5676;Point(1 1 1)'))$$),
  ('3d tIntersects at the plane z=0', 'false', $$asText(tIntersects(z1, geometry 'SRID=5676;Point(1 1 0)'))$$),
  ('3d tIntersects bodies same', 'all true', $$asText(tIntersects(z1, z1))$$),
  ('2d tIntersects bodies same', 'all true', $$asText(tIntersects(xy, xy))$$),
  ('3d tDisjoint same z', 'false then true', $$asText(tDisjoint(z1, geometry 'SRID=5676;Point(1 1 1)'))$$),
  ('2d aDisjoint far', 'true', $$aDisjoint(xy, geometry 'SRID=5676;Point(1 50)')$$),
  ('2d eContains', 'true', $$eContains(geometry 'SRID=5676;Polygon((-9 -9,9 -9,9 9,-9 9,-9 -9))', xy)$$),
  ('2d convexHull', 'polygon', $$asEWKT(convexHull(xy))$$),
  ('3d convexHull', 'keeps z', $$asEWKT(convexHull(z1))$$),
  ('2d atStbox other x', 'NULL', $$asText(atStbox(xy, stbox 'SRID=5676;STBOX XT(((40,40),(50,50)),[2001-01-01,2001-01-03])'))$$),
  ('3d atStbox other x', 'NULL', $$asText(atStbox(z1, stbox 'SRID=5676;STBOX ZT(((40,40,0),(50,50,2)),[2001-01-01,2001-01-03])'))$$),
  ('3d atStbox wide z', 'whole value', $$asText(atStbox(z1, stbox 'SRID=5676;STBOX ZT(((0,0,0),(50,50,2)),[2001-01-01,2001-01-03])'))$$),
  ('3d eTouches on edge same z', 'true', $$eTouches(z1, geometry 'SRID=5676;Point(0 0 1)')$$),
  ('3d eIntersects z=1 box', 'true', $$eIntersects(z1, geometry 'SRID=5676;Point(1 1 1)')$$),
  ('3d stbox', 'tight 0..7, 0..2, z=1', $$z1::stbox::text$$),
  ('2d stbox', 'tight 0..7, 0..2', $$xy::stbox::text$$);
DO $$
DECLARE r record; v text; msg text;
BEGIN
  FOR r IN SELECT * FROM q LOOP
    BEGIN
      EXECUTE 'SELECT (' || r.sql || ')::text FROM t' INTO v;
      RAISE NOTICE 'K|%|expected %|got %', r.name, r.expected, coalesce(left(v, 170), 'NULL');
    EXCEPTION WHEN OTHERS THEN
      GET STACKED DIAGNOSTICS msg = MESSAGE_TEXT;
      RAISE NOTICE 'K|%|expected %|ERROR %', r.name, r.expected, msg;
    END;
  END LOOP;
END $$;
'''

if sys.argv[1] == 'sql':
    print({'ledger': LEDGER, 'known': KNOWN, 'controls': CONTROLS}[sys.argv[2]])
    sys.exit(0)

O = sys.argv[2]
calls = {}
for l in open(O, errors='replace'):
    if 'NOTICE:' not in l:
        continue
    l = l.split('NOTICE:', 1)[1].strip()
    if l.startswith('OK|'):
        _, fn, lab = l.split('|', 2)
        calls[(fn, lab)] = None
    elif l.startswith('ERR|'):
        _, fn, lab, st, msg = l.split('|', 4)
        calls[(fn, lab)] = re.sub(r'\b\d+(\.\d+)?\b', 'N', msg)

gaps = collections.defaultdict(lambda: collections.defaultdict(set))
for (fn, lab), msg in calls.items():
    if msg is None:
        continue
    p = lab.split(' ')
    for i, x in enumerate(p):
        cands = []
        if '/' in x:
            b, m = x.split('/')
            if b not in ('convex', 'convex3d'):
                cands.append((f'body {b}', f'{"convex3d" if b.endswith("3d") else "convex"}/{m}'))
            if b.endswith('3d'):
                cands.append(('dimension 3D', f'{b[:-2]}/{m}'))
            if m == 'rotate':
                cands.append(('motion rotate', f'{b}/translate'))
            if m in ('discrete', 'step', 'inst'):
                cands.append((f'motion {m}', f'{b}/translate'))
        elif x.startswith('g-') and x != 'g-convex':
            cands.append((f'geometry {x[2:]}', 'g-convex'))
        for what, base in cands:
            q = list(p)
            q[i] = base
            k = (fn, ' '.join(q))
            if k in calls and calls[k] is None:
                gaps[(what, msg)][fn.split('(')[0]].add(fn)

for (what, msg), fns in sorted(gaps.items(), key=lambda kv: (kv[0][0], kv[0][1])):
    print(f'{what:20} | {msg[:110]}')
    print('     ', ' '.join(sorted(fns))[:600])
print()
print('functions asked', len({f for f, _ in calls}))
threeD = collections.defaultdict(lambda: [0, 0])
for (fn, lab), msg in calls.items():
    if '3d/' in lab.split(' ')[0]:
        threeD[fn.split('(')[0]][0 if msg is None else 1] += 1
print('3D first argument, functions with every call failing:')
print('     ', ' '.join(sorted(k for k, v in threeD.items() if v[0] == 0))[:900])
