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
-- tContains
-------------------------------------------------------------------------------

-- Test for NULL inputs since the function is not STRICT
SELECT tContains(NULL::geometry, tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tContains(geometry 'Point(1 1)', NULL::tcbuffer);

SELECT tContains(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tContains(geometry 'Point(1 1)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}');
SELECT tContains(geometry 'Point(1 1)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]');
SELECT tContains(geometry 'Point(1 1)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}');

SELECT tContains(geometry 'Point empty', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tContains(geometry 'Point empty', tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}');
SELECT tContains(geometry 'Point empty', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]');
SELECT tContains(geometry 'Point empty', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}');

SELECT tContains(geometry 'Linestring(1 1,2 2)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02]');

-- The disc leaves the geometry: containment stops at the internal tangency,
-- when the disc starts poking out, not at the later external tangency where it
-- stops intersecting
SELECT tContains(geometry 'Polygon((-5 -5,-5 5,5 5,5 -5,-5 -5))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(8 0),0.5)@2001-01-03]');
SELECT tContains(geometry 'Polygon((-5 -5,-5 5,5 5,5 -5,-5 -5))', tcbuffer '[Cbuffer(Point(8 0),0.5)@2001-01-01, Cbuffer(Point(0 0),0.5)@2001-01-03]');
-- A hole the disc passes over: contained on both sides, not while it overlaps
-- the hole, so containment starts and stops at internal tangencies only
SELECT tContains(geometry 'Polygon((-5 -5,-5 5,5 5,5 -5,-5 -5),(-1 -1,-1 1,1 1,1 -1,-1 -1))', tcbuffer '[Cbuffer(Point(-3 0),0.5)@2001-01-01, Cbuffer(Point(3 0),0.5)@2001-01-03]');
SELECT tCovers(geometry 'Polygon((-5 -5,-5 5,5 5,5 -5,-5 -5),(-1 -1,-1 1,1 1,1 -1,-1 -1))', tcbuffer '[Cbuffer(Point(-3 0),0.5)@2001-01-01, Cbuffer(Point(3 0),0.5)@2001-01-03]');

-- Curve polygon (arc ring) input, native arc-exact; the ring is the circle of
-- centre (0,0) and radius 5. The disc stays strictly inside the ring
SELECT tContains(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(3 0),0.5)@2001-01-03]');
-- The disc leaves the arc ring at the internal tangency
SELECT tContains(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(8 0),0.5)@2001-01-03]');
-- Internally tangent disc: covered by the ring but not strictly contained
SELECT tContains(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer 'Cbuffer(Point(4.5 0),0.5)@2001-01-01');
-- Multi surface grouping the same curve polygon
SELECT tContains(geometry 'MultiSurface(CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0)))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(3 0),0.5)@2001-01-03]');
SELECT tContains(geometry 'MultiSurface(CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0)))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(8 0),0.5)@2001-01-03]');

/* Errors */
SELECT tContains(geometry 'SRID=5676;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tContains(geometry 'Point(1 1)', tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tContains(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2001-01-01');
SELECT tContains(geometry 'Point(1 1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');

-- Two temporal circular buffers: a small disc passing straight through a large
-- stationary one is contained on the interval between the two clearance
-- crossings, not only at the closest-approach instant
SELECT tContains(tcbuffer '[Cbuffer(Point(0 0),5)@2001-01-01, Cbuffer(Point(0 0),5)@2001-01-03]', tcbuffer '[Cbuffer(Point(-10 0),1)@2001-01-01, Cbuffer(Point(10 0),1)@2001-01-03]');
SELECT tCovers(tcbuffer '[Cbuffer(Point(0 0),5)@2001-01-01, Cbuffer(Point(0 0),5)@2001-01-03]', tcbuffer '[Cbuffer(Point(-10 0),1)@2001-01-01, Cbuffer(Point(10 0),1)@2001-01-03]');
-- Identical moving buffers cover each other throughout but never strictly
-- contain (the boundaries coincide)
SELECT tContains(tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(4 0),1)@2001-01-03]', tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(4 0),1)@2001-01-03]');
SELECT tCovers(tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(4 0),1)@2001-01-03]', tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(4 0),1)@2001-01-03]');

-------------------------------------------------------------------------------
-- tDisjoint
-------------------------------------------------------------------------------

-- Test for NULL inputs since the function is not STRICT
SELECT tDisjoint(NULL::geometry, tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tDisjoint(geometry 'Point(1 1)', NULL::tcbuffer);
SELECT tDisjoint(NULL::tcbuffer, geometry 'Point(1 1)');
SELECT tDisjoint(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', NULL::geometry);
SELECT tDisjoint(NULL::tcbuffer, tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tDisjoint(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', NULL::tcbuffer);

SELECT tDisjoint(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tDisjoint(geometry 'Point(1 1)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}');
SELECT tDisjoint(geometry 'Point(1 1)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]');
SELECT tDisjoint(geometry 'Point(1 1)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}');

SELECT tDisjoint(geometry 'Point empty', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tDisjoint(geometry 'Point empty', tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}');
SELECT tDisjoint(geometry 'Point empty', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]');
SELECT tDisjoint(geometry 'Point empty', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}');

SELECT tDisjoint(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'Point(1 1)');
SELECT tDisjoint(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', geometry 'Point(1 1)');
SELECT tDisjoint(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', geometry 'Point(1 1)');
SELECT tDisjoint(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', geometry 'Point(1 1)');

SELECT tDisjoint(tcbuffer '[Cbuffer(Point(0 1),0.5)@2001-01-01, Cbuffer(Point(2 1),0.5)@2001-01-04]', geometry 'Linestring(1 0,1 1,2 1,2 0)');
SELECT tDisjoint(tcbuffer '[Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-04]', geometry 'Linestring(1 1,2 1)');
SELECT tDisjoint(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(0 0),0.5)@2001-01-04]', geometry 'Linestring(0 0,1 1)');

SELECT tDisjoint(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'Point empty');
SELECT tDisjoint(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', geometry 'Point empty');
SELECT tDisjoint(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', geometry 'Point empty');
SELECT tDisjoint(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', geometry 'Point empty');

-- Circular arc input (native arc-exact); arc centre (0,0) radius 5, upper-right
-- quadrant. Complement of the corresponding tIntersects cases below
SELECT tDisjoint(geometry 'CircularString(5 0, 4 3, 0 5)', tcbuffer '[Cbuffer(Point(8 6),2.5)@2001-01-01, Cbuffer(Point(4 3),2.5)@2001-01-03]');
SELECT tDisjoint(tcbuffer '[Cbuffer(Point(4 3),2.5)@2001-01-01, Cbuffer(Point(8 6),2.5)@2001-01-03]', geometry 'CircularString(5 0, 4 3, 0 5)');
-- Off-span: the disc meets the full circle but not the arc (angular gating)
SELECT tDisjoint(geometry 'CircularString(5 0, 4 3, 0 5)', tcbuffer '[Cbuffer(Point(5 -3),0.5)@2001-01-01, Cbuffer(Point(5 -1),0.5)@2001-01-03]');
-- Compound curve (arc chained to a line)
SELECT tDisjoint(geometry 'CompoundCurve(CircularString(5 0, 4 3, 0 5),(0 5, -4 5))', tcbuffer '[Cbuffer(Point(-2 8),0.5)@2001-01-01, Cbuffer(Point(-2 4),0.5)@2001-01-03]');
-- Curve polygon (arc ring): the disc starts inside the ring and exits it
SELECT tDisjoint(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(8 0),0.5)@2001-01-03]');

/* Errors */
SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tDisjoint(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'SRID=5676;Point(1 1)');
-- 3D geometry (cbuffer is 2D only)
SELECT tDisjoint(tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2001-01-01', geometry 'Point(2 2 2)');
SELECT tDisjoint(tcbuffer '{Cbuffer(Point(1 1 1),0.5)@2001-01-01, Cbuffer(Point(2 2 2),0.5)@2001-01-02, Cbuffer(Point(1 1 1),0.5)@2001-01-03}', geometry 'Point(2 2 2)');
SELECT tDisjoint(tcbuffer '[Cbuffer(Point(1 1 1),0.5)@2001-01-01, Cbuffer(Point(2 2 2),0.5)@2001-01-02, Cbuffer(Point(1 1 1),0.5)@2001-01-03]', geometry 'Point(2 2 2)');
SELECT tDisjoint(tcbuffer '{[Cbuffer(Point(1 1 1),0.5)@2001-01-01, Cbuffer(Point(2 2 2),0.5)@2001-01-02, Cbuffer(Point(1 1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3 3),0.5)@2001-01-04, Cbuffer(Point(3 3 3),0.5)@2001-01-05]}', geometry 'Point(2 2 2)');

-------------------------------------------------------------------------------
-- tIntersects
-------------------------------------------------------------------------------

-- Test for NULL inputs since the function is not STRICT
SELECT tIntersects(NULL::geometry, tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tIntersects(geometry 'Point(1 1)', NULL::tcbuffer);
SELECT tIntersects(NULL::tcbuffer, geometry 'Point(1 1)');
SELECT tIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', NULL::geometry);
SELECT tIntersects(NULL::tcbuffer, tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', NULL::tcbuffer);

SELECT tIntersects(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tIntersects(geometry 'Point(1 1)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}');
SELECT tIntersects(geometry 'Point(1 1)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]');
SELECT tIntersects(geometry 'Point(1 1)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}');

SELECT tIntersects(geometry 'Point empty', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tIntersects(geometry 'Point empty', tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}');
SELECT tIntersects(geometry 'Point empty', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]');
SELECT tIntersects(geometry 'Point empty', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}');

SELECT tIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'Point(1 1)');
SELECT tIntersects(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', geometry 'Point(1 1)');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', geometry 'Point(1 1)');
SELECT tIntersects(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', geometry 'Point(1 1)');

SELECT tIntersects(tcbuffer '[Cbuffer(Point(0 1),0.5)@2001-01-01, Cbuffer(Point(2 1),0.5)@2001-01-04]', geometry 'Linestring(1 0,1 1,2 1,2 0)');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-04]', geometry 'Linestring(1 1,2 1)');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(0 0),0.5)@2001-01-04]', geometry 'Linestring(0 0,1 1)');

SELECT tIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'Point empty');
SELECT tIntersects(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', geometry 'Point empty');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', geometry 'Point empty');
SELECT tIntersects(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', geometry 'Point empty');

SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02]', geometry 'Linestring(1 1,2 2)');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(4 1),0.5)@2001-01-02]', geometry 'Linestring(1 2,1 0,2 0,2 2)');

-- Circular arc input (native arc-exact); arc centre (0,0) radius 5, upper-right quadrant
SELECT tIntersects(geometry 'CircularString(5 0, 4 3, 0 5)', tcbuffer '[Cbuffer(Point(8 6),2.5)@2001-01-01, Cbuffer(Point(4 3),2.5)@2001-01-03]');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(4 3),2.5)@2001-01-01, Cbuffer(Point(8 6),2.5)@2001-01-03]', geometry 'CircularString(5 0, 4 3, 0 5)');
-- Off-span: the disc meets the full circle but not the arc (angular gating)
SELECT tIntersects(geometry 'CircularString(5 0, 4 3, 0 5)', tcbuffer '[Cbuffer(Point(5 -3),0.5)@2001-01-01, Cbuffer(Point(5 -1),0.5)@2001-01-03]');
-- Compound curve (arc chained to a line): the arc component drives the same
-- result as the arc-only case, and the line component is walked too
SELECT tIntersects(geometry 'CompoundCurve(CircularString(5 0, 4 3, 0 5),(0 5, -4 5))', tcbuffer '[Cbuffer(Point(8 6),2.5)@2001-01-01, Cbuffer(Point(4 3),2.5)@2001-01-03]');
SELECT tIntersects(geometry 'CompoundCurve(CircularString(5 0, 4 3, 0 5),(0 5, -4 5))', tcbuffer '[Cbuffer(Point(-2 8),0.5)@2001-01-01, Cbuffer(Point(-2 4),0.5)@2001-01-03]');
-- Curve polygon (arc ring): the disc starts inside the ring and exits it
SELECT tIntersects(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(8 0),0.5)@2001-01-03]');
-- Multi curve and multi surface grouping the same arc components
SELECT tIntersects(geometry 'MultiCurve(CircularString(5 0, 4 3, 0 5),(0 5, -4 5))', tcbuffer '[Cbuffer(Point(8 6),2.5)@2001-01-01, Cbuffer(Point(4 3),2.5)@2001-01-03]');
SELECT tIntersects(geometry 'MultiSurface(CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0)))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(8 0),0.5)@2001-01-03]');

-- Coverage
SELECT tIntersects(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-03}', tcbuffer 'Cbuffer(Point(2 2),0.5)@2001-01-02');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02]', tcbuffer '[Cbuffer(Point(2 1),0.5)@2001-01-01, Cbuffer(Point(1 2),0.5)@2001-01-02]');

/* Errors */
SELECT tIntersects(geometry 'SRID=5676;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'SRID=5676;Point(1 1)');
-- 3D geometry (cbuffer is 2D only)
SELECT tIntersects(tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2001-01-01', geometry 'Point(2 2 2)');
SELECT tIntersects(tcbuffer '{Cbuffer(Point(1 1 1),0.5)@2001-01-01, Cbuffer(Point(2 2 2),0.5)@2001-01-02, Cbuffer(Point(1 1 1),0.5)@2001-01-03}', geometry 'Point(2 2 2)');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1 1),0.5)@2001-01-01, Cbuffer(Point(2 2 2),0.5)@2001-01-02, Cbuffer(Point(1 1 1),0.5)@2001-01-03]', geometry 'Point(2 2 2)');
SELECT tIntersects(tcbuffer '{[Cbuffer(Point(1 1 1),0.5)@2001-01-01, Cbuffer(Point(2 2 2),0.5)@2001-01-02, Cbuffer(Point(1 1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3 3),0.5)@2001-01-04, Cbuffer(Point(3 3 3),0.5)@2001-01-05]}', geometry 'Point(2 2 2)');

-------------------------------------------------------------------------------
-- tTouches
-------------------------------------------------------------------------------
-- The function does not support 3D or geographies

-- Test for NULL inputs since the function is not STRICT
SELECT tTouches(NULL::geometry, tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tTouches(geometry 'Point(1 1)', NULL::tcbuffer);

SELECT tTouches(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tTouches(geometry 'Point(1 1)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}');
SELECT tTouches(geometry 'Point(1 1)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]');
SELECT tTouches(geometry 'Point(1 1)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}');

SELECT tTouches(geometry 'Point empty', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tTouches(geometry 'Point empty', tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}');
SELECT tTouches(geometry 'Point empty', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]');
SELECT tTouches(geometry 'Point empty', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}');

-- Test for NULL inputs since the function is not STRICT
SELECT tTouches(NULL::tcbuffer, geometry 'Point(1 1)');
SELECT tTouches(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', NULL::geometry);

SELECT tTouches(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'Point(1 1)');
SELECT tTouches(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', geometry 'Point(1 1)');
SELECT tTouches(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', geometry 'Point(1 1)');
SELECT tTouches(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', geometry 'Point(1 1)');

SELECT tTouches(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'Point empty');
SELECT tTouches(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', geometry 'Point empty');
SELECT tTouches(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', geometry 'Point empty');
SELECT tTouches(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', geometry 'Point empty');

SELECT tTouches(geometry 'Linestring(1 1,2 2)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02]');

-- Curve polygon (arc ring) input, native arc-exact; the ring is the circle of
-- centre (0,0) and radius 5. The disc of radius 1 comes from outside and is
-- externally tangent to the ring at the instant its centre is at distance 6
SELECT tTouches(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(10 0),1)@2001-01-01, Cbuffer(Point(0 0),1)@2001-01-03]');
-- The disc stays outside the ring and never reaches it
SELECT tTouches(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(10 0),1)@2001-01-01, Cbuffer(Point(10 10),1)@2001-01-03]');
-- Circular arc (a curve, not a region): the disc grazes the arc
SELECT tTouches(geometry 'CircularString(5 0, 4 3, 0 5)', tcbuffer '[Cbuffer(Point(10 0),1)@2001-01-01, Cbuffer(Point(0 0),1)@2001-01-03]');

/* Errors */
SELECT tTouches(geometry 'SRID=5676;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tTouches(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT tTouches(geometry 'Point(1 1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01');
SELECT tTouches(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2001-01-01');

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

-- Test for NULL inputs since the function is not STRICT
SELECT tDwithin(NULL::geometry, tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', 2);
SELECT tDwithin(geometry 'Point(1 1)', NULL::tcbuffer, 2);
SELECT tDwithin(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', NULL);

SELECT tDwithin(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', 2);
SELECT tDwithin(geometry 'Point(1 1)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', 2);
SELECT tDwithin(geometry 'Point(1 1)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', 2);
SELECT tDwithin(geometry 'Point(1 1)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', 2);

SELECT tDwithin(geometry 'Point empty', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', 2);
SELECT tDwithin(geometry 'Point empty', tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', 2);
SELECT tDwithin(geometry 'Point empty', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', 2);
SELECT tDwithin(geometry 'Point empty', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', 2);

-- Test for NULL inputs since the function is not STRICT
SELECT tDwithin(NULL::tcbuffer, geometry 'Point(1 1)', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', NULL::geometry, 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'Point(1 1)', NULL);

SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'Point(1 1)', 2);
SELECT tDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', geometry 'Point(1 1)', 2);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', geometry 'Point(1 1)', 2);
SELECT tDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', geometry 'Point(1 1)', 2);

SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'Point empty', 2);
SELECT tDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', geometry 'Point empty', 2);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', geometry 'Point empty', 2);
SELECT tDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', geometry 'Point empty', 2);

-- Test for NULL inputs since the function is not STRICT
SELECT tDwithin(NULL::tcbuffer, tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', NULL::tcbuffer, 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', NULL);

-- Coverage
SELECT tDwithin(tcbuffer '(Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02]', geometry 'Point(0 1)', 1);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(1 2),0.5)@2001-01-03]', geometry 'Point(2 3)', 1);

SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', 2);
SELECT tDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', 2);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', 2);
SELECT tDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', 2);
SELECT tDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', 2);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', 2);
SELECT tDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', 2);
SELECT tDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', 2);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', 2);
SELECT tDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', 2);
SELECT tDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', 2);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03]', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', 2);
SELECT tDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03], [Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', 2);

SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(1 3),0.5)@2001-01-03]', geometry 'Point(1 2)', 0);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(1 2),0.5)@2001-01-03]', geometry 'Point(1 3)', 0);
SELECT tDwithin(tcbuffer '(Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02]', tcbuffer '[Cbuffer(Point(1 0),0.5)@2001-01-01, Cbuffer(Point(2 0),0.5)@2001-01-02]', 1);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02]', tcbuffer '[Cbuffer(Point(1 0),0.5)@2001-01-01, Cbuffer(Point(2 0),0.5)@2001-01-02]', 1);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(0 1),0.5)@2001-01-01, Cbuffer(Point(0 0),0.5)@2001-01-02]', tcbuffer '[Cbuffer(Point(2 0),0.5)@2001-01-01, Cbuffer(Point(1 0),0.5)@2001-01-02]', 1);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(0 0),0.5)@2001-01-02]', tcbuffer '[Cbuffer(Point(2 0),0.5)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02]', 1);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02]', tcbuffer '[Cbuffer(Point(0 2),0.5)@2001-01-01, Cbuffer(Point(1 3),0.5)@2001-01-02]', 1);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02]', tcbuffer '[Cbuffer(Point(4 0),0.5)@2001-01-01, Cbuffer(Point(3 1),0.5)@2001-01-02]', 0);

SELECT tDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03, Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}', 1);

SELECT tDwithin(geometry 'Linestring(1 1,2 2)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'Linestring(1 1,2 2)', 2);

-- Circular arc input (native arc-exact); arc centre (0,0) radius 5, upper-right quadrant
SELECT tDwithin(geometry 'CircularString(5 0, 4 3, 0 5)', tcbuffer '[Cbuffer(Point(8 6),0.5)@2001-01-01, Cbuffer(Point(4 3),0.5)@2001-01-03]', 2);
-- Moving radius (dr != 0): the growing disc reaches within-distance of the arc
SELECT tDwithin(geometry 'CircularString(5 0, 4 3, 0 5)', tcbuffer '[Cbuffer(Point(10 0),0.5)@2001-01-01, Cbuffer(Point(10 0),4.5)@2001-01-03]', 2.5);
-- Off-span: within the full circle but not the arc (angular gating)
SELECT tDwithin(geometry 'CircularString(5 0, 4 3, 0 5)', tcbuffer '[Cbuffer(Point(5 -3),0.5)@2001-01-01, Cbuffer(Point(5 -1),0.5)@2001-01-03]', 0.3);

/* Errors */
SELECT tDwithin(geometry 'SRID=5676;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'SRID=5676;Point(1 1)', 2);
SELECT tDwithin(tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', geometry 'Point(0 0)', -1);
-- 3D geometry (cbuffer is 2D only)
SELECT tDwithin(geometry 'Point(1 1 1)', tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2001-01-01', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2001-01-01', geometry 'Point(1 1 1)', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2001-01-01', tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2001-01-01', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2001-01-01', tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', 2);

-------------------------------------------------------------------------------
-- Discs of a zero radius: the value is a temporal point
--
-- A disc of a zero radius is the point at its centre, so the answers below are
-- those of the temporal point the value converts to. The contact is at a zero
-- clearance, which is the disc boundary the disc kernels read a touch from
-------------------------------------------------------------------------------

-- A point running through the position of a point geometry meets it at the
-- single instant it passes through
SELECT tIntersects(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]', geometry 'Point(0 0)');
SELECT tIntersects(geometry 'Point(0 0)', tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]');
SELECT tDisjoint(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]', geometry 'Point(0 0)');
-- The same point crossing a line, and running along it
SELECT tIntersects(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]', geometry 'Linestring(0 -5,0 5)');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]', geometry 'Linestring(-5 0,5 0)');
-- Entering and leaving a polygon
SELECT tIntersects(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]', geometry 'Polygon((-1 -1,-1 1,1 1,1 -1,-1 -1))');
SELECT tDisjoint(geometry 'Polygon((-1 -1,-1 1,1 1,1 -1,-1 -1))', tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]');
-- A point standing still on the position of a point geometry
SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1),0)@2001-01-01, Cbuffer(Point(1 1),0)@2001-01-03]', geometry 'Point(1 1)');
-- Within a distance of a line the point approaches and leaves
SELECT tDwithin(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]', geometry 'Linestring(0 -5,0 5)', 2);
SELECT tDwithin(geometry 'Linestring(0 -5,0 5)', tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]', 2);
-- A point touches a geometry where it lies on the boundary of the geometry,
-- so it touches a polygon it crosses at the two boundary crossings
SELECT tTouches(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]', geometry 'Polygon((-1 -1,-1 1,1 1,1 -1,-1 -1))');
SELECT tTouches(geometry 'Polygon((-1 -1,-1 1,1 1,1 -1,-1 -1))', tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]');
-- A geometry whose boundary is empty is touched nowhere, even at the instant
-- the point passes through it
SELECT tTouches(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]', geometry 'Point(0 0)');
SELECT tTouches(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]', geometry 'MultiPoint(0 0,1 1)');
-- The boundary of a line is its two end points, which a point crossing the
-- line never reaches
SELECT tTouches(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]', geometry 'Linestring(0 -5,0 5)');
-- A point reaching an end point of the line touches it there
SELECT tTouches(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(5 0),0)@2001-01-03]', geometry 'Linestring(-5 0,5 0)');
-- A geometry contains a point where the point lies in its open interior, and
-- covers it where the point lies in the closed geometry, so the two differ at
-- the boundary crossings
SELECT tContains(geometry 'Polygon((-1 -1,-1 1,1 1,1 -1,-1 -1))', tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]');
SELECT tCovers(geometry 'Polygon((-1 -1,-1 1,1 1,1 -1,-1 -1))', tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]');
-- A geometry whose boundary is empty contains the point wherever it covers it
SELECT tContains(geometry 'Point(0 0)', tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]');
SELECT tCovers(geometry 'Point(0 0)', tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]');
-- A line covers the point it carries, and contains it away from its end points
SELECT tCovers(geometry 'Linestring(-5 0,5 0)', tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]');
SELECT tContains(geometry 'Linestring(-5 0,5 0)', tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(5 0),0)@2001-01-03]');
-- Two points that coincide do not touch, since their interiors coincide,
-- while each contains and covers the other
SELECT tTouches(tcbuffer 'Cbuffer(Point(0 0),0)@2001-01-01', tcbuffer 'Cbuffer(Point(0 0),0)@2001-01-01');
SELECT tContains(tcbuffer 'Cbuffer(Point(0 0),0)@2001-01-01', tcbuffer 'Cbuffer(Point(0 0),0)@2001-01-01');
SELECT tCovers(tcbuffer 'Cbuffer(Point(0 0),0)@2001-01-01', tcbuffer 'Cbuffer(Point(0 0),0)@2001-01-01');
SELECT tTouches(tcbuffer 'Cbuffer(Point(0 0),0)@2001-01-01', tcbuffer 'Cbuffer(Point(5 5),0)@2001-01-01');
SELECT tContains(tcbuffer 'Cbuffer(Point(0 0),0)@2001-01-01', tcbuffer 'Cbuffer(Point(5 5),0)@2001-01-01');
-- Two points crossing meet at a single instant, which they contain and cover
-- each other at and touch at nowhere
SELECT tTouches(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]', tcbuffer '[Cbuffer(Point(0 -3),0)@2001-01-01, Cbuffer(Point(0 3),0)@2001-01-03]');
SELECT tContains(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]', tcbuffer '[Cbuffer(Point(0 -3),0)@2001-01-01, Cbuffer(Point(0 3),0)@2001-01-03]');
SELECT tCovers(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0)@2001-01-03]', tcbuffer '[Cbuffer(Point(0 -3),0)@2001-01-01, Cbuffer(Point(0 3),0)@2001-01-03]');
-- Discs of a strictly positive radius keep the disc kernels
SELECT tTouches(tcbuffer 'Cbuffer(Point(0 0),1)@2001-01-01', tcbuffer 'Cbuffer(Point(2 0),1)@2001-01-01');
SELECT tCovers(tcbuffer 'Cbuffer(Point(0 0),2)@2001-01-01', tcbuffer 'Cbuffer(Point(0 0),1)@2001-01-01');
-- A single disc of a strictly positive radius keeps the disc semantics
SELECT tIntersects(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0.5)@2001-01-03]', geometry 'Point(0 0)');
SELECT tContains(geometry 'Polygon((-1 -1,-1 1,1 1,1 -1,-1 -1))', tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0.5)@2001-01-03]');
SELECT tTouches(tcbuffer '[Cbuffer(Point(-3 0),0)@2001-01-01, Cbuffer(Point(3 0),0.5)@2001-01-03]', geometry 'Polygon((-1 -1,-1 1,1 1,1 -1,-1 -1))');

-------------------------------------------------------------------------------
-- The relationship holds against the geometry, not against a circle enclosing
-- it: the farthest point of the geometry from the centre of the disc lies off
-- the line joining the two centres, so a circle enclosing the geometry reaches
-- outside the disc while the geometry itself does not
-------------------------------------------------------------------------------

SELECT tCovers(tcbuffer 'Cbuffer(Point(0 0),3.2)@2001-01-01', geometry 'Linestring(3 0,3 1)');
SELECT tContains(tcbuffer 'Cbuffer(Point(0 0),3.2)@2001-01-01', geometry 'Linestring(3 0,3 1)');
-- The same value against a disc that the geometry reaches outside of
SELECT tCovers(tcbuffer 'Cbuffer(Point(0 0),3)@2001-01-01', geometry 'Linestring(3 0,3 1)');
-- A disc that stops covering the geometry as it moves away
SELECT tCovers(tcbuffer '{Cbuffer(Point(0 0),3.2)@2001-01-01, Cbuffer(Point(9 9),1)@2001-01-03}', geometry 'Linestring(3 0,3 1)');
SELECT tCovers(tcbuffer '{[Cbuffer(Point(0 0),3.2)@2001-01-01, Cbuffer(Point(0 0),3.2)@2001-01-02],[Cbuffer(Point(9 9),1)@2001-01-04, Cbuffer(Point(9 9),1)@2001-01-05]}', geometry 'Linestring(3 0,3 1)');
-- A disc touching a line it is tangent to
SELECT tTouches(tcbuffer 'Cbuffer(Point(0 0),1)@2001-01-01', geometry 'Linestring(1 -5,1 5)');

-------------------------------------------------------------------------------
