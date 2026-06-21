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
SELECT tContains(NULL::geometry, tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tContains(geometry 'Point(1 1)', NULL::tcbuffer);

SELECT tContains(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tContains(geometry 'Point(1 1)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT tContains(geometry 'Point(1 1)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT tContains(geometry 'Point(1 1)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

SELECT tContains(geometry 'Point empty', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tContains(geometry 'Point empty', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT tContains(geometry 'Point empty', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT tContains(geometry 'Point empty', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

SELECT tContains(geometry 'Linestring(1 1,2 2)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02]');

/* Errors */
SELECT tContains(geometry 'SRID=5676;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tContains(geometry 'Point(1 1)', tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tContains(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2000-01-01');
SELECT tContains(geometry 'Point(1 1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');

-------------------------------------------------------------------------------
-- tDisjoint
-------------------------------------------------------------------------------

-- Test for NULL inputs since the function is not STRICT
SELECT tDisjoint(NULL::geometry, tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tDisjoint(geometry 'Point(1 1)', NULL::tcbuffer);
SELECT tDisjoint(NULL::tcbuffer, geometry 'Point(1 1)');
SELECT tDisjoint(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', NULL::geometry);
SELECT tDisjoint(NULL::tcbuffer, tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tDisjoint(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', NULL::tcbuffer);

SELECT tDisjoint(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tDisjoint(geometry 'Point(1 1)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT tDisjoint(geometry 'Point(1 1)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT tDisjoint(geometry 'Point(1 1)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

SELECT tDisjoint(geometry 'Point empty', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tDisjoint(geometry 'Point empty', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT tDisjoint(geometry 'Point empty', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT tDisjoint(geometry 'Point empty', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

SELECT tDisjoint(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'Point(1 1)');
SELECT tDisjoint(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', geometry 'Point(1 1)');
SELECT tDisjoint(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', geometry 'Point(1 1)');
SELECT tDisjoint(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', geometry 'Point(1 1)');

SELECT tDisjoint(tcbuffer '[Cbuffer(Point(0 1),0.5)@2000-01-01, Cbuffer(Point(2 1),0.5)@2000-01-04]', geometry 'Linestring(1 0,1 1,2 1,2 0)');
SELECT tDisjoint(tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-04]', geometry 'Linestring(1 1,2 1)');
SELECT tDisjoint(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(0 0),0.5)@2000-01-04]', geometry 'Linestring(0 0,1 1)');

SELECT tDisjoint(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'Point empty');
SELECT tDisjoint(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', geometry 'Point empty');
SELECT tDisjoint(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', geometry 'Point empty');
SELECT tDisjoint(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', geometry 'Point empty');

/* Errors */
SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tDisjoint(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
-- 3D geometry (cbuffer is 2D only)
SELECT tDisjoint(tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2000-01-01', geometry 'Point(2 2 2)');
SELECT tDisjoint(tcbuffer '{Cbuffer(Point(1 1 1),0.5)@2000-01-01, Cbuffer(Point(2 2 2),0.5)@2000-01-02, Cbuffer(Point(1 1 1),0.5)@2000-01-03}', geometry 'Point(2 2 2)');
SELECT tDisjoint(tcbuffer '[Cbuffer(Point(1 1 1),0.5)@2000-01-01, Cbuffer(Point(2 2 2),0.5)@2000-01-02, Cbuffer(Point(1 1 1),0.5)@2000-01-03]', geometry 'Point(2 2 2)');
SELECT tDisjoint(tcbuffer '{[Cbuffer(Point(1 1 1),0.5)@2000-01-01, Cbuffer(Point(2 2 2),0.5)@2000-01-02, Cbuffer(Point(1 1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3 3),0.5)@2000-01-04, Cbuffer(Point(3 3 3),0.5)@2000-01-05]}', geometry 'Point(2 2 2)');

-------------------------------------------------------------------------------
-- tIntersects
-------------------------------------------------------------------------------

-- Test for NULL inputs since the function is not STRICT
SELECT tIntersects(NULL::geometry, tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tIntersects(geometry 'Point(1 1)', NULL::tcbuffer);
SELECT tIntersects(NULL::tcbuffer, geometry 'Point(1 1)');
SELECT tIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', NULL::geometry);
SELECT tIntersects(NULL::tcbuffer, tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', NULL::tcbuffer);

SELECT tIntersects(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tIntersects(geometry 'Point(1 1)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT tIntersects(geometry 'Point(1 1)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT tIntersects(geometry 'Point(1 1)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

SELECT tIntersects(geometry 'Point empty', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tIntersects(geometry 'Point empty', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT tIntersects(geometry 'Point empty', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT tIntersects(geometry 'Point empty', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

SELECT tIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'Point(1 1)');
SELECT tIntersects(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', geometry 'Point(1 1)');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', geometry 'Point(1 1)');
SELECT tIntersects(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', geometry 'Point(1 1)');

SELECT tIntersects(tcbuffer '[Cbuffer(Point(0 1),0.5)@2000-01-01, Cbuffer(Point(2 1),0.5)@2000-01-04]', geometry 'Linestring(1 0,1 1,2 1,2 0)');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-04]', geometry 'Linestring(1 1,2 1)');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(0 0),0.5)@2000-01-04]', geometry 'Linestring(0 0,1 1)');

SELECT tIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'Point empty');
SELECT tIntersects(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', geometry 'Point empty');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', geometry 'Point empty');
SELECT tIntersects(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', geometry 'Point empty');

SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02]', geometry 'Linestring(1 1,2 2)');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(4 1),0.5)@2000-01-02]', geometry 'Linestring(1 2,1 0,2 0,2 2)');

-- Coverage
SELECT tIntersects(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-03}', tcbuffer 'Cbuffer(Point(2 2),0.5)@2000-01-02');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02]', tcbuffer '[Cbuffer(Point(2 1),0.5)@2000-01-01, Cbuffer(Point(1 2),0.5)@2000-01-02]');

/* Errors */
SELECT tIntersects(geometry 'SRID=5676;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tIntersects(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
-- 3D geometry (cbuffer is 2D only)
SELECT tIntersects(tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2000-01-01', geometry 'Point(2 2 2)');
SELECT tIntersects(tcbuffer '{Cbuffer(Point(1 1 1),0.5)@2000-01-01, Cbuffer(Point(2 2 2),0.5)@2000-01-02, Cbuffer(Point(1 1 1),0.5)@2000-01-03}', geometry 'Point(2 2 2)');
SELECT tIntersects(tcbuffer '[Cbuffer(Point(1 1 1),0.5)@2000-01-01, Cbuffer(Point(2 2 2),0.5)@2000-01-02, Cbuffer(Point(1 1 1),0.5)@2000-01-03]', geometry 'Point(2 2 2)');
SELECT tIntersects(tcbuffer '{[Cbuffer(Point(1 1 1),0.5)@2000-01-01, Cbuffer(Point(2 2 2),0.5)@2000-01-02, Cbuffer(Point(1 1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3 3),0.5)@2000-01-04, Cbuffer(Point(3 3 3),0.5)@2000-01-05]}', geometry 'Point(2 2 2)');

-------------------------------------------------------------------------------
-- tTouches
-------------------------------------------------------------------------------
-- The function does not support 3D or geographies

-- Test for NULL inputs since the function is not STRICT
SELECT tTouches(NULL::geometry, tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tTouches(geometry 'Point(1 1)', NULL::tcbuffer);

SELECT tTouches(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tTouches(geometry 'Point(1 1)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT tTouches(geometry 'Point(1 1)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT tTouches(geometry 'Point(1 1)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

SELECT tTouches(geometry 'Point empty', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tTouches(geometry 'Point empty', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT tTouches(geometry 'Point empty', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT tTouches(geometry 'Point empty', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

-- Test for NULL inputs since the function is not STRICT
SELECT tTouches(NULL::tcbuffer, geometry 'Point(1 1)');
SELECT tTouches(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', NULL::geometry);

SELECT tTouches(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'Point(1 1)');
SELECT tTouches(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', geometry 'Point(1 1)');
SELECT tTouches(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', geometry 'Point(1 1)');
SELECT tTouches(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', geometry 'Point(1 1)');

SELECT tTouches(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'Point empty');
SELECT tTouches(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', geometry 'Point empty');
SELECT tTouches(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', geometry 'Point empty');
SELECT tTouches(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', geometry 'Point empty');

SELECT tTouches(geometry 'Linestring(1 1,2 2)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02]');

/* Errors */
SELECT tTouches(geometry 'SRID=5676;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tTouches(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT tTouches(geometry 'Point(1 1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT tTouches(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2000-01-01');

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

-- Test for NULL inputs since the function is not STRICT
SELECT tDwithin(NULL::geometry, tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT tDwithin(geometry 'Point(1 1)', NULL::tcbuffer, 2);
SELECT tDwithin(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', NULL);

SELECT tDwithin(geometry 'Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT tDwithin(geometry 'Point(1 1)', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', 2);
SELECT tDwithin(geometry 'Point(1 1)', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', 2);
SELECT tDwithin(geometry 'Point(1 1)', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', 2);

SELECT tDwithin(geometry 'Point empty', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT tDwithin(geometry 'Point empty', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', 2);
SELECT tDwithin(geometry 'Point empty', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', 2);
SELECT tDwithin(geometry 'Point empty', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', 2);

-- Test for NULL inputs since the function is not STRICT
SELECT tDwithin(NULL::tcbuffer, geometry 'Point(1 1)', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', NULL::geometry, 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'Point(1 1)', NULL);

SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'Point(1 1)', 2);
SELECT tDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', geometry 'Point(1 1)', 2);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', geometry 'Point(1 1)', 2);
SELECT tDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', geometry 'Point(1 1)', 2);

SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'Point empty', 2);
SELECT tDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', geometry 'Point empty', 2);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', geometry 'Point empty', 2);
SELECT tDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', geometry 'Point empty', 2);

-- Test for NULL inputs since the function is not STRICT
SELECT tDwithin(NULL::tcbuffer, tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', NULL::tcbuffer, 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', NULL);

-- Coverage
SELECT tDwithin(tcbuffer '(Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02]', geometry 'Point(0 1)', 1);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(1 2),0.5)@2000-01-03]', geometry 'Point(2 3)', 1);

SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT tDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT tDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', 2);
SELECT tDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', 2);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', 2);
SELECT tDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', 2);
SELECT tDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', 2);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', 2);
SELECT tDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', 2);
SELECT tDwithin(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', 2);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', 2);
SELECT tDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', 2);

SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(1 3),0.5)@2000-01-03]', geometry 'Point(1 2)', 0);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(1 2),0.5)@2000-01-03]', geometry 'Point(1 3)', 0);
SELECT tDwithin(tcbuffer '(Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02]', tcbuffer '[Cbuffer(Point(1 0),0.5)@2000-01-01, Cbuffer(Point(2 0),0.5)@2000-01-02]', 1);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02]', tcbuffer '[Cbuffer(Point(1 0),0.5)@2000-01-01, Cbuffer(Point(2 0),0.5)@2000-01-02]', 1);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(0 1),0.5)@2000-01-01, Cbuffer(Point(0 0),0.5)@2000-01-02]', tcbuffer '[Cbuffer(Point(2 0),0.5)@2000-01-01, Cbuffer(Point(1 0),0.5)@2000-01-02]', 1);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(0 0),0.5)@2000-01-02]', tcbuffer '[Cbuffer(Point(2 0),0.5)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02]', 1);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02]', tcbuffer '[Cbuffer(Point(0 2),0.5)@2000-01-01, Cbuffer(Point(1 3),0.5)@2000-01-02]', 1);
SELECT tDwithin(tcbuffer '[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02]', tcbuffer '[Cbuffer(Point(4 0),0.5)@2000-01-01, Cbuffer(Point(3 1),0.5)@2000-01-02]', 0);

SELECT tDwithin(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03, Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', 1);

SELECT tDwithin(geometry 'Linestring(1 1,2 2)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'Linestring(1 1,2 2)', 2);

/* Errors */
SELECT tDwithin(geometry 'SRID=5676;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'SRID=5676;Point(1 1)', 2);
SELECT tDwithin(tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'Point(0 0)', -1);
-- 3D geometry (cbuffer is 2D only)
SELECT tDwithin(geometry 'Point(1 1 1)', tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2000-01-01', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2000-01-01', geometry 'Point(1 1 1)', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2000-01-01', tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2000-01-01', 2);
SELECT tDwithin(tcbuffer 'Cbuffer(Point(1 1 1),0.5)@2000-01-01', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);

-------------------------------------------------------------------------------
