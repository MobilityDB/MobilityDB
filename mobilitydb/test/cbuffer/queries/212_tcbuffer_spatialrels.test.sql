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
-- eContains
-------------------------------------------------------------------------------

SELECT eContains(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT eContains(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT eContains(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT eContains(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

SELECT eContains(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eContains(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eContains(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eContains(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', cbuffer 'Cbuffer(Point(1 1),0.5)');

SELECT eContains(tcbuffer '[Cbuffer(Point(4 2),0.5)@2000-01-01, Cbuffer(Point(2 4),0.5)@2000-01-02]', geometry 'Linestring(1 1,3 3)');
SELECT eContains(tcbuffer '[Cbuffer(Point(4 2),0.5)@2000-01-01, Cbuffer(Point(2 4),0.5)@2000-01-02]', geometry 'Linestring(1 1,3 3,1 1)');
SELECT eContains(tcbuffer '[Cbuffer(Point(0 1),0.5)@2000-01-01, Cbuffer(Point(4 1),0.5)@2000-01-02]', geometry 'Polygon((1 1,1 3,3 3,3 1,1 1))');
SELECT eContains(tcbuffer '[Cbuffer(Point(1 4),0.5)@2000-01-01, Cbuffer(Point(4 1),0.5)@2000-01-02]', geometry 'Polygon((1 1,1 3,3 3,3 1,1 1))');

-- A hole the disc passes over: it is contained on both sides of the hole, so
-- the ever semantics hold and the always semantics do not, both decided at
-- internal tangencies where the disc starts and stops overlapping the hole
SELECT eContains(geometry 'Polygon((-5 -5,-5 5,5 5,5 -5,-5 -5),(-1 -1,-1 1,1 1,1 -1,-1 -1))', tcbuffer '[Cbuffer(Point(-3 0),0.5)@2000-01-01, Cbuffer(Point(3 0),0.5)@2000-01-03]');
SELECT aContains(geometry 'Polygon((-5 -5,-5 5,5 5,5 -5,-5 -5),(-1 -1,-1 1,1 1,1 -1,-1 -1))', tcbuffer '[Cbuffer(Point(-3 0),0.5)@2000-01-01, Cbuffer(Point(3 0),0.5)@2000-01-03]');
-- The disc is contained only strictly inside the segment, at no vertex
SELECT eContains(geometry 'Polygon((-5 -5,-5 5,5 5,5 -5,-5 -5))', tcbuffer '[Cbuffer(Point(-8 0),0.5)@2000-01-01, Cbuffer(Point(8 0),0.5)@2000-01-03]');

-- Curve polygon (arc ring) input, native arc-exact; the ring is the circle of
-- centre (0,0) and radius 5. The disc starts strictly inside and leaves it
SELECT eContains(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(8 0),0.5)@2000-01-03]');
SELECT aContains(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(8 0),0.5)@2000-01-03]');
-- The disc stays strictly inside the ring
SELECT aContains(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(3 0),0.5)@2000-01-03]');
-- Internally tangent disc: covered by the ring but not strictly contained
SELECT eContains(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer 'Cbuffer(Point(4.5 0),0.5)@2000-01-01');

SELECT eContains(tcbuffer 'Cbuffer(Point(1 1),1)@2000-01-01', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT eContains(tcbuffer '[Cbuffer(Point(1 1),1)@2000-01-01, Cbuffer(Point(2 2),1)@2000-01-02]', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02]');

/* Errors */
SELECT eContains(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'SRID=3812;Point(1 1)');

-------------------------------------------------------------------------------
-- eCovers
-------------------------------------------------------------------------------

SELECT eCovers(cbuffer 'Cbuffer(Point(1 1),1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT eCovers(cbuffer 'Cbuffer(Point(1 1),1)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT eCovers(cbuffer 'Cbuffer(Point(1 1),1)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT eCovers(cbuffer 'Cbuffer(Point(1 1),1)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

SELECT eCovers(tcbuffer 'Cbuffer(Point(1 1),1)@2000-01-01', cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eCovers(tcbuffer '{Cbuffer(Point(1 1),1)@2000-01-01, Cbuffer(Point(2 2),1)@2000-01-02, Cbuffer(Point(1 1),1)@2000-01-03}', cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eCovers(tcbuffer '[Cbuffer(Point(1 1),1)@2000-01-01, Cbuffer(Point(2 2),1)@2000-01-02, Cbuffer(Point(1 1),1)@2000-01-03]', cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eCovers(tcbuffer '{[Cbuffer(Point(1 1),1)@2000-01-01, Cbuffer(Point(2 2),1)@2000-01-02, Cbuffer(Point(1 1),1)@2000-01-03],[Cbuffer(Point(3 3),1)@2000-01-04, Cbuffer(Point(3 3),1)@2000-01-05]}', cbuffer 'Cbuffer(Point(1 1),0.5)');

SELECT eCovers(tcbuffer 'Cbuffer(Point(1 1),1)@2000-01-01', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT eCovers(tcbuffer '[Cbuffer(Point(1 1),1)@2000-01-01, Cbuffer(Point(2 2),1)@2000-01-02]', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02]');

-- Curve polygon (arc ring) input, native arc-exact; the ring is the circle of
-- centre (0,0) and radius 5. The internally tangent disc is covered but not
-- strictly contained, unlike the eContains case above
SELECT eCovers(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer 'Cbuffer(Point(4.5 0),0.5)@2000-01-01');
SELECT eCovers(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(8 0),0.5)@2000-01-03]');
SELECT aCovers(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(3 0),0.5)@2000-01-03]');

/* Errors */
SELECT eCovers(tcbuffer 'Cbuffer(Point(1 1),1)@2000-01-01', geometry 'SRID=3812;Point(1 1)');

-------------------------------------------------------------------------------
-- eDisjoint
-------------------------------------------------------------------------------

SELECT eDisjoint(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT eDisjoint(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT eDisjoint(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT eDisjoint(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

SELECT eDisjoint(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01',  cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eDisjoint(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}',  cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eDisjoint(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]',  cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eDisjoint(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}',  cbuffer 'Cbuffer(Point(1 1),0.5)');

-- Curved input, native arc-exact; the curve polygon ring is the circle of
-- centre (0,0) and radius 5. The disc starts inside the ring and exits it
SELECT eDisjoint(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(8 0),0.5)@2000-01-03]');
-- The disc stays inside the ring
SELECT eDisjoint(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(3 0),0.5)@2000-01-03]');
-- The disc stays outside the ring
SELECT aDisjoint(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(8 0),0.5)@2000-01-01, Cbuffer(Point(8 8),0.5)@2000-01-03]');
-- Circular arc, off-span: the disc meets the full circle but not the arc
SELECT aDisjoint(geometry 'CircularString(5 0, 4 3, 0 5)', tcbuffer '[Cbuffer(Point(5 -3),0.5)@2000-01-01, Cbuffer(Point(5 -1),0.5)@2000-01-03]');

/* Errors */
SELECT eDisjoint(geometry 'SRID=3812;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT eDisjoint(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'SRID=3812;Point(1 1)');

-------------------------------------------------------------------------------
-- eIntersects
-------------------------------------------------------------------------------

------------------------
-- Geo x Temporal
------------------------

SELECT eIntersects(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT eIntersects(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT eIntersects(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT eIntersects(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

------------------------
-- Temporal x Geo
------------------------
SELECT eIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01',  cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eIntersects(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}',  cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]',  cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eIntersects(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}',  cbuffer 'Cbuffer(Point(1 1),0.5)');

------------------------
-- Temporal x Temporal
------------------------
-- Temporal x Instant
SELECT eIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT eIntersects(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT eIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT eIntersects(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
-- Temporal x Discrete Sequence
SELECT eIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT eIntersects(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT eIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT eIntersects(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
-- Temporal x Continuous Sequence
SELECT eIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT eIntersects(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT eIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT eIntersects(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
-- Temporal x SequenceSet
SELECT eIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');
SELECT eIntersects(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');
SELECT eIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');
SELECT eIntersects(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

------------------------
-- Curved geometry input (native arc-exact)
------------------------
-- Arc centre (0,0) radius 5, upper-right quadrant
SELECT eIntersects(geometry 'CircularString(5 0, 4 3, 0 5)', tcbuffer '[Cbuffer(Point(8 6),2.5)@2000-01-01, Cbuffer(Point(4 3),2.5)@2000-01-03]');
-- Off-span: the disc meets the full circle but not the arc (angular gating)
SELECT eIntersects(geometry 'CircularString(5 0, 4 3, 0 5)', tcbuffer '[Cbuffer(Point(5 -3),0.5)@2000-01-01, Cbuffer(Point(5 -1),0.5)@2000-01-03]');
-- Compound curve (arc chained to a line) and the multi types grouping them
SELECT eIntersects(geometry 'CompoundCurve(CircularString(5 0, 4 3, 0 5),(0 5, -4 5))', tcbuffer '[Cbuffer(Point(-2 8),0.5)@2000-01-01, Cbuffer(Point(-2 4),0.5)@2000-01-03]');
SELECT eIntersects(geometry 'MultiCurve(CircularString(5 0, 4 3, 0 5),(0 5, -4 5))', tcbuffer '[Cbuffer(Point(8 6),2.5)@2000-01-01, Cbuffer(Point(4 3),2.5)@2000-01-03]');
SELECT eIntersects(geometry 'MultiSurface(CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0)))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(8 0),0.5)@2000-01-03]');
-- The disc stays inside the curve polygon ring
SELECT aIntersects(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(3 0),0.5)@2000-01-03]');
SELECT aIntersects(tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(8 0),0.5)@2000-01-03]', geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))');

/* Errors */
SELECT eIntersects(geometry 'SRID=3812;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT eIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'SRID=3812;Point(1 1)');
SELECT eIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer 'SRID=3812;Cbuffer(Point(1 1),0.5)@2000-01-01');

-------------------------------------------------------------------------------
-- eTouches
-------------------------------------------------------------------------------

SELECT eTouches(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT eTouches(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT eTouches(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT eTouches(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

SELECT eTouches(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01',  cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eTouches(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}',  cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eTouches(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]',  cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT eTouches(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}',  cbuffer 'Cbuffer(Point(1 1),0.5)');

SELECT eTouches(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');
SELECT eTouches(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');
SELECT eTouches(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}',  cbuffer 'Cbuffer(Point(1 1),0.5)');

-- tcbuffer x tcbuffer (circles touch when distance between centers = sum of radii)
SELECT eTouches(tcbuffer 'Cbuffer(Point(0 0),0.5)@2000-01-01', tcbuffer 'Cbuffer(Point(1 0),0.5)@2000-01-01');
SELECT eTouches(tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(1 0),0.5)@2000-01-02]', tcbuffer '[Cbuffer(Point(1 0),0.5)@2000-01-01, Cbuffer(Point(2 0),0.5)@2000-01-02]');

-- Curve polygon (arc ring) input, native arc-exact; the ring is the circle of
-- centre (0,0) and radius 5. The disc of radius 1 is externally tangent to the
-- ring when its centre is at distance 6
SELECT eTouches(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer 'Cbuffer(Point(6 0),1)@2000-01-01');
SELECT eTouches(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(10 0),1)@2000-01-01, Cbuffer(Point(0 0),1)@2000-01-03]');
SELECT aTouches(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer 'Cbuffer(Point(6 0),1)@2000-01-01');
-- The disc stays away from the ring
SELECT eTouches(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(10 0),1)@2000-01-01, Cbuffer(Point(10 10),1)@2000-01-03]');

/* Errors */
SELECT eTouches(geometry 'SRID=3812;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT eTouches(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'SRID=3812;Point(1 1)');
-- unsupported geometry type
SELECT eTouches(tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02]',
  geometry 'POLYHEDRALSURFACE( ((0 0 0, 0 0 1, 0 1 1, 0 1 0, 0 0 0)),
  ((0 0 0, 0 1 0, 1 1 0, 1 0 0, 0 0 0)), ((0 0 0, 1 0 0, 1 0 1, 0 0 1, 0 0 0)),
  ((1 1 0, 1 1 1, 1 0 1, 1 0 0, 1 1 0)),
  ((0 1 0, 0 1 1, 1 1 1, 1 1 0, 0 1 0)), ((0 0 1, 1 0 1, 1 1 1, 0 1 1, 0 0 1)) )');

-------------------------------------------------------------------------------
-- eDwithin
-------------------------------------------------------------------------------

SELECT eDwithin(geometry 'Linestring(1 1,2 2)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT eDwithin(geometry 'Linestring(1 1,2 2)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', 2);
SELECT eDwithin(geometry 'Linestring(1 1,2 2)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', 2);
SELECT eDwithin(geometry 'Linestring(1 1,2 2)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', 2);

SELECT eDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT eDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT eDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT eDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT eDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', 2);
SELECT eDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', 2);
SELECT eDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', 2);
SELECT eDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', 2);
SELECT eDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', 2);
SELECT eDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', 2);
SELECT eDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', 2);
SELECT eDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', 2);
SELECT eDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', 2);
SELECT eDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', 2);
SELECT eDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', 2);
SELECT eDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', 2);

SELECT eDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-02]', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-03}', 10);
SELECT eDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(2 2),0.5)@2000-01-03, Cbuffer(Point(1 1),0.5)@2000-01-05]', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-04, Cbuffer(Point(2 2),0.5)@2000-01-06}', 10);
SELECT eDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02]', tcbuffer '[Cbuffer(Point(2 2),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02]', 2);
SELECT eDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(0 0),0.5)@2000-01-02]', tcbuffer '[Cbuffer(Point(0 2),0.5)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02]', 2);
SELECT eDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(2 2),0.5)@2000-01-03],[Cbuffer(Point(1 1),0.5)@2000-01-05]}', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-04}', 10);
SELECT eDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(2 2),0.5)@2000-01-03],[Cbuffer(Point(1 1),0.5)@2000-01-06]}', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-04, Cbuffer(Point(2 2),0.5)@2000-01-05]', 10);
SELECT eDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(2 2),0.5)@2000-01-03],[Cbuffer(Point(1 1),0.5)@2000-01-06]}', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01],[Cbuffer(Point(1 1),0.5)@2000-01-04, Cbuffer(Point(2 2),0.5)@2000-01-05]}', 10);

-- Step interpolation
SELECT eDwithin(tcbuffer 'Interp=Step;{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', tcbuffer 'Interp=Step;{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', 2);

-- Curved input (native arc-exact); arc centre (0,0) radius 5, upper-right
-- quadrant. The distance is folded into the disc radius by the same kernel
SELECT eDwithin(geometry 'CircularString(5 0, 4 3, 0 5)', tcbuffer '[Cbuffer(Point(8 6),0.5)@2000-01-01, Cbuffer(Point(4 3),0.5)@2000-01-03]', 2);
-- Off-span: within the full circle but not the arc (angular gating)
SELECT eDwithin(geometry 'CircularString(5 0, 4 3, 0 5)', tcbuffer '[Cbuffer(Point(5 -3),0.5)@2000-01-01, Cbuffer(Point(5 -1),0.5)@2000-01-03]', 0.3);
-- The disc stays inside the curve polygon ring, hence within any distance
SELECT aDwithin(geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(3 0),0.5)@2000-01-03]', 1);
SELECT aDwithin(tcbuffer '[Cbuffer(Point(8 0),0.5)@2000-01-01, Cbuffer(Point(8 8),0.5)@2000-01-03]', geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', 1);

/* Errors */
SELECT eDwithin(geometry 'SRID=3812;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT eDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'SRID=3812;Point(1 1)', 2);
SELECT eDwithin(tcbuffer 'SRID=3812;Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);

-------------------------------------------------------------------------------
