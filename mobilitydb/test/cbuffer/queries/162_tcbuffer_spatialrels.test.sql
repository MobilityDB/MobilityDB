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

SELECT eContains(geometry 'Linestring(1 1,3 3)', tcbuffer '[Cbuffer(Point(4 2),0.5)@2000-01-01, Cbuffer(Point(2 4),0.5)@2000-01-02]');
SELECT eContains(geometry 'Linestring(1 1,3 3,1 1)', tcbuffer '[Cbuffer(Point(4 2),0.5)@2000-01-01, Cbuffer(Point(2 4),0.5)@2000-01-02]');
SELECT eContains(geometry 'Polygon((1 1,1 3,3 3,3 1,1 1))', tcbuffer '[Cbuffer(Point(0 1),0.5)@2000-01-01, Cbuffer(Point(4 1),0.5)@2000-01-02]');
SELECT eContains(geometry 'Polygon((1 1,1 3,3 3,3 1,1 1))', tcbuffer '[Cbuffer(Point(1 4),0.5)@2000-01-01, Cbuffer(Point(4 1),0.5)@2000-01-02]');

SELECT eContains(tcbuffer '[Cbuffer(Point(4 2),0.5)@2000-01-01, Cbuffer(Point(2 4),0.5)@2000-01-02]', geometry 'Linestring(1 1,3 3)');
SELECT eContains(tcbuffer '[Cbuffer(Point(4 2),0.5)@2000-01-01, Cbuffer(Point(2 4),0.5)@2000-01-02]', geometry 'Linestring(1 1,3 3,1 1)');
SELECT eContains(tcbuffer '[Cbuffer(Point(0 1),0.5)@2000-01-01, Cbuffer(Point(4 1),0.5)@2000-01-02]', geometry 'Polygon((1 1,1 3,3 3,3 1,1 1))');
SELECT eContains(tcbuffer '[Cbuffer(Point(1 4),0.5)@2000-01-01, Cbuffer(Point(4 1),0.5)@2000-01-02]', geometry 'Polygon((1 1,1 3,3 3,3 1,1 1))');

/* Errors */
SELECT eContains(geometry 'SRID=3812;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');

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

/* Errors */
SELECT eDwithin(geometry 'SRID=3812;Point(1 1)', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);
SELECT eDwithin(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'SRID=3812;Point(1 1)', 2);
SELECT eDwithin(tcbuffer 'SRID=3812;Cbuffer(Point(1 1),0.5)@2000-01-01', tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 2);

-------------------------------------------------------------------------------
