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

SELECT round(geometry 'Point(1 1)' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(geometry 'Point(1 1)' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(geometry 'Point(1 1)' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(geometry 'Point(1 1)' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);

SELECT round(geometry 'Point empty' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(geometry 'Point empty' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(geometry 'Point empty' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(geometry 'Point empty' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);

SELECT round(cbuffer 'Cbuffer(Point(1 1), 0.2)' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(cbuffer 'Cbuffer(Point(1 1), 0.2)' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(cbuffer 'Cbuffer(Point(1 1), 0.2)' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(cbuffer 'Cbuffer(Point(1 1), 0.2)' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);

SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> cbuffer 'Cbuffer(Point(1 1), 0.2)', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> cbuffer 'Cbuffer(Point(1 1), 0.2)', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> cbuffer 'Cbuffer(Point(1 1), 0.2)', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> cbuffer 'Cbuffer(Point(1 1), 0.2)', 6);

SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> geometry 'Point(1 1)', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> geometry 'Point(1 1)', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> geometry 'Point(1 1)', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> geometry 'Point(1 1)', 6);

SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> geometry 'Point empty', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> geometry 'Point empty', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> geometry 'Point empty', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> geometry 'Point empty', 6);

SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);

-------------------------------------------------------------------------------


-- Analytic nearest approach distance (|=| and nearestApproachDistance):
-- polygons, lines, holes, multi, containing polygon (interior ray-cast),
-- and a curved type (exact traversed-area fallback)
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))', 6);
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Polygon((-3 -3,-3 3,3 3,3 -3,-3 -3))', 6);
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Polygon((-10 -10,-10 10,10 10,10 -10,-10 -10))', 6);
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Linestring(4 -3,4 6)', 6);
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Circularstring(5 0,7 2,9 0)', 6);
SELECT round(geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))' |=| tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', 6);
SELECT round(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]' |=| geometry 'Polygon((20 20,20 24,24 24,24 20,20 20))', 6);
SELECT round(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]' |=| geometry 'Linestring(20 -5,20 20)', 6);
SELECT round(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]' |=| geometry 'Multipolygon(((200 200,200 210,210 210,210 200,200 200)),((9 -1,9 1,12 1,12 -1,9 -1)))', 6);
SELECT round(tcbuffer 'Interp=Step;[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(5 5), 3)@2000-01-02]' |=| geometry 'Polygon((11 -1,11 3,14 3,14 -1,11 -1))', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(4 0), 1)@2000-01-02], [Cbuffer(Point(20 20), 2)@2000-01-03, Cbuffer(Point(25 20), 1)@2000-01-04]}' |=| geometry 'Polygon((-5 -5,-5 15,15 15,15 -5,-5 -5),(0 0,4 0,4 4,0 4,0 0))', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(4 0), 1)@2000-01-02], [Cbuffer(Point(20 20), 2)@2000-01-03, Cbuffer(Point(25 20), 1)@2000-01-04]}' |=| geometry 'Multilinestring((50 50,60 60),(2 -3,2 9))', 6);
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]', geometry 'Polygon((11 -1,11 3,14 3,14 -1,11 -1))'), 6);
SELECT round(nearestApproachDistance(geometry 'Linestring(4 -3,4 6)', tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01'), 6);
SELECT round(nearestApproachDistance(tcbuffer '{Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(8 3), 2)@2000-01-02}', geometry 'Multipolygon(((200 200,200 210,210 210,210 200,200 200)),((9 -1,9 1,12 1,12 -1,9 -1)))'), 6);

-------------------------------------------------------------------------------

-- Analytic shortestLine: its length equals the nearest-approach distance;
-- exercises the witness path over polygons, lines, hole, multi, a
-- containing polygon (degenerate) and a curved type (exact fallback)
SELECT geometrytype(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))'));
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Linestring(4 -3,4 6)'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Polygon((-2 -2,-2 30,30 30,30 -2,-2 -2))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Circularstring(5 0,7 2,9 0)'))::numeric, 6);
SELECT round(ST_Length(shortestLine(geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))', tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]', geometry 'Polygon((20 20,20 24,24 24,24 20,20 20))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]', geometry 'Multipolygon(((200 200,200 210,210 210,210 200,200 200)),((9 -1,9 1,12 1,12 -1,9 -1)))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer 'Interp=Step;[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(5 5), 3)@2000-01-02]', geometry 'Polygon((11 -1,11 3,14 3,14 -1,11 -1))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer '{[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(4 0), 1)@2000-01-02], [Cbuffer(Point(20 20), 2)@2000-01-03, Cbuffer(Point(25 20), 1)@2000-01-04]}', geometry 'Polygon((-5 -5,-5 15,15 15,15 -5,-5 -5),(0 0,4 0,4 4,0 4,0 0))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer '{Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(8 3), 2)@2000-01-02}', geometry 'Multipolygon(((200 200,200 210,210 210,210 200,200 200)),((9 -1,9 1,12 1,12 -1,9 -1)))'))::numeric, 6);

-------------------------------------------------------------------------------
