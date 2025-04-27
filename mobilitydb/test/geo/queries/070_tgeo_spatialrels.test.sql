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

SELECT eContains(geometry 'Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT eContains(geometry 'Point(1 1)', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eContains(geometry 'Point(1 1)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eContains(geometry 'Point(1 1)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT eContains(geometry 'Linestring(1 1,3 3)', tgeometry '[Point(4 2)@2000-01-01, Point(2 4)@2000-01-02]');
SELECT eContains(geometry 'Linestring(1 1,3 3,1 1)', tgeometry '[Point(4 2)@2000-01-01, Point(2 4)@2000-01-02]');
SELECT eContains(geometry 'Polygon((1 1,1 3,3 3,3 1,1 1))', tgeometry '[Point(0 1)@2000-01-01, Point(4 1)@2000-01-02]');
SELECT eContains(geometry 'Polygon((1 1,1 3,3 3,3 1,1 1))', tgeometry '[Point(1 4)@2000-01-01, Point(4 1)@2000-01-02]');

SELECT eContains(geometry 'Point empty', tgeometry 'Point(1 1)@2000-01-01');
SELECT eContains(geometry 'Point empty', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eContains(geometry 'Point empty', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eContains(geometry 'Point empty', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

/* Errors */
SELECT eContains(geometry 'SRID=3812;Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT eContains(geometry 'Point(1 1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT eContains(geometry 'Point(1 1)', tgeometry 'Point(1 1 1)@2000-01-01');

-------------------------------------------------------------------------------
-- eDisjoint
-------------------------------------------------------------------------------

SELECT eDisjoint(geometry 'Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT eDisjoint(geometry 'Point(1 1)', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eDisjoint(geometry 'Point(1 1)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eDisjoint(geometry 'Point(1 1)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT eDisjoint(geometry 'Point empty', tgeometry 'Point(1 1)@2000-01-01');
SELECT eDisjoint(geometry 'Point empty', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eDisjoint(geometry 'Point empty', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eDisjoint(geometry 'Point empty', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT eDisjoint(geometry 'Point(1 1 1)', tgeometry 'Point(1 1 1)@2000-01-01');
SELECT eDisjoint(geometry 'Point(1 1 1)', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT eDisjoint(geometry 'Point(1 1 1)', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT eDisjoint(geometry 'Point(1 1 1)', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT eDisjoint(geometry 'Point Z empty', tgeometry 'Point(1 1 1)@2000-01-01');
SELECT eDisjoint(geometry 'Point Z empty', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT eDisjoint(geometry 'Point Z empty', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT eDisjoint(geometry 'Point Z empty', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT eDisjoint(tgeometry 'Point(1 1)@2000-01-01',  geometry 'Point(1 1)');
SELECT eDisjoint(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point(1 1)');
SELECT eDisjoint(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point(1 1)');
SELECT eDisjoint(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');

SELECT eDisjoint(tgeometry 'Point(1 1)@2000-01-01',  geometry 'Point empty');
SELECT eDisjoint(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point empty');
SELECT eDisjoint(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point empty');
SELECT eDisjoint(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point empty');

SELECT eDisjoint(tgeometry 'Point(1 1 1)@2000-01-01',  geometry 'Point(1 1 1)');
SELECT eDisjoint(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point(1 1 1)');
SELECT eDisjoint(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point(1 1 1)');
SELECT eDisjoint(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point(1 1 1)');

SELECT eDisjoint(tgeometry 'Point(1 1 1)@2000-01-01',  geometry 'Point Z empty');
SELECT eDisjoint(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point Z empty');
SELECT eDisjoint(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point Z empty');
SELECT eDisjoint(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point Z empty');

SELECT eDisjoint(geometry 'Point(1 1)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT eDisjoint(geometry 'Point(1 1 1)', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT eDisjoint(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');
SELECT eDisjoint(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point(1 1 1)');

-- Mixed 2D/3D
SELECT eDisjoint(geometry 'Point(1 1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT eDisjoint(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1)');

-- Coverage
SELECT eDisjoint(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))', tgeometry '{Point(0 0)@2000-01-01, Point(0 2)@2000-01-02}');
SELECT eDisjoint(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))', tgeometry '[Point(0 0)@2000-01-01, Point(0 2)@2000-01-02]');
SELECT eDisjoint(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))', tgeometry '{[Point(0 0)@2000-01-01, Point(0 2)@2000-01-02],[Point(0 2)@2000-01-03, Point(0 0)@2000-01-04]}');
SELECT eDisjoint(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))', tgeometry '{Point(3 3)@2000-01-01, Point(4 4)@2000-01-02}');

/* Errors */
SELECT eDisjoint(geometry 'SRID=3812;Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT eDisjoint(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=3812;Point(1 1)');

-------------------------------------------------------------------------------
-- eIntersects
-------------------------------------------------------------------------------

------------------------
-- Geo x Temporal
------------------------
-- 2D
SELECT eIntersects(geometry 'Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT eIntersects(geometry 'Point(1 1)', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eIntersects(geometry 'Point(1 1)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eIntersects(geometry 'Point(1 1)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
-- Empty
SELECT eIntersects(geometry 'Point empty', tgeometry 'Point(1 1)@2000-01-01');
SELECT eIntersects(geometry 'Point empty', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eIntersects(geometry 'Point empty', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eIntersects(geometry 'Point empty', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
-- 3D
SELECT eIntersects(geometry 'Point(1 1 1)', tgeometry 'Point(1 1 1)@2000-01-01');
SELECT eIntersects(geometry 'Point(1 1 1)', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT eIntersects(geometry 'Point(1 1 1)', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT eIntersects(geometry 'Point(1 1 1)', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
-- Empty
SELECT eIntersects(geometry 'Point Z empty', tgeometry 'Point(1 1 1)@2000-01-01');
SELECT eIntersects(geometry 'Point Z empty', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT eIntersects(geometry 'Point Z empty', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT eIntersects(geometry 'Point Z empty', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

-- Geography
SELECT eIntersects(geography 'Point(1.5 1.5)', tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT eIntersects(geography 'Point(1.5 1.5)', tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT eIntersects(geography 'Point(1.5 1.5)', tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT eIntersects(geography 'Point(1.5 1.5)', tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
-- Empty
SELECT eIntersects(geography 'Point empty', tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT eIntersects(geography 'Point empty', tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT eIntersects(geography 'Point empty', tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT eIntersects(geography 'Point empty', tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
-- 3D
SELECT eIntersects(geography 'Point(1.5 1.5 1.5)', tgeography 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT eIntersects(geography 'Point(1.5 1.5 1.5)', tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT eIntersects(geography 'Point(1.5 1.5 1.5)', tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT eIntersects(geography 'Point(1.5 1.5 1.5)', tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');
-- Empty
SELECT eIntersects(geography 'Point Z empty', tgeography 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT eIntersects(geography 'Point Z empty', tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT eIntersects(geography 'Point Z empty', tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT eIntersects(geography 'Point Z empty', tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

------------------------
-- Temporal x Geo
------------------------
SELECT eIntersects(tgeometry 'Point(1 1)@2000-01-01',  geometry 'Point(1 1)');
SELECT eIntersects(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point(1 1)');
SELECT eIntersects(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point(1 1)');
SELECT eIntersects(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');
-- Empty
SELECT eIntersects(tgeometry 'Point(1 1)@2000-01-01',  geometry 'Point empty');
SELECT eIntersects(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point empty');
SELECT eIntersects(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point empty');
SELECT eIntersects(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point empty');
-- 3D
SELECT eIntersects(tgeometry 'Point(1 1 1)@2000-01-01',  geometry 'Point(1 1 1)');
SELECT eIntersects(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point(1 1 1)');
SELECT eIntersects(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point(1 1 1)');
SELECT eIntersects(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point(1 1 1)');
-- Empty
SELECT eIntersects(tgeometry 'Point(1 1 1)@2000-01-01',  geometry 'Point Z empty');
SELECT eIntersects(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point Z empty');
SELECT eIntersects(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point Z empty');
SELECT eIntersects(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point Z empty');

-- Geography
SELECT eIntersects(tgeography 'Point(1.5 1.5)@2000-01-01',  geography 'Point(1.5 1.5)');
SELECT eIntersects(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}',  geography 'Point(1.5 1.5)');
SELECT eIntersects(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]',  geography 'Point(1.5 1.5)');
SELECT eIntersects(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}',  geography 'Point(1.5 1.5)');
-- Empty
SELECT eIntersects(tgeography 'Point(1.5 1.5)@2000-01-01',  geography 'Point empty');
SELECT eIntersects(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}',  geography 'Point empty');
SELECT eIntersects(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]',  geography 'Point empty');
SELECT eIntersects(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}',  geography 'Point empty');
-- 3D
SELECT eIntersects(tgeography 'Point(1.5 1.5 1.5)@2000-01-01',  geography 'Point(1.5 1.5 1.5)');
SELECT eIntersects(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}',  geography 'Point(1.5 1.5 1.5)');
SELECT eIntersects(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]',  geography 'Point(1.5 1.5 1.5)');
SELECT eIntersects(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}',  geography 'Point(1.5 1.5 1.5)');
-- Empty
SELECT eIntersects(tgeography 'Point(1.5 1.5 1.5)@2000-01-01',  geography 'Point Z empty');
SELECT eIntersects(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}',  geography 'Point Z empty');
SELECT eIntersects(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]',  geography 'Point Z empty');
SELECT eIntersects(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}',  geography 'Point Z empty');

------------------------
-- Temporal x Temporal
------------------------
-- Temporal x Instant
SELECT eIntersects(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-01');
SELECT eIntersects(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry 'Point(1 1)@2000-01-01');
SELECT eIntersects(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry 'Point(1 1)@2000-01-01');
SELECT eIntersects(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry 'Point(1 1)@2000-01-01');
-- Temporal x Discrete Sequence
SELECT eIntersects(tgeometry 'Point(1 1)@2000-01-01', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eIntersects(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eIntersects(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eIntersects(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
-- Temporal x Continuous Sequence
SELECT eIntersects(tgeometry 'Point(1 1)@2000-01-01', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eIntersects(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eIntersects(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eIntersects(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
-- Temporal x SequenceSet
SELECT eIntersects(tgeometry 'Point(1 1)@2000-01-01', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT eIntersects(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT eIntersects(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT eIntersects(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

-- Mixed 2D/3D
SELECT eIntersects(geometry 'Point(1 1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT eIntersects(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1)');

SELECT eIntersects(geography 'Point(1.5 1.5 1.5)', tgeography 'Point(1 1)@2000-01-01');
SELECT eIntersects(tgeography 'Point(1 1 1)@2000-01-01', geography 'Point(1.5 1.5)');

-- Coverage
SELECT eIntersects(tgeometry '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-03}', tgeometry 'Point(1 1)@2000-01-02');
SELECT eIntersects(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(1 1)@2000-01-04, Point(2 2)@2000-01-05]}', tgeometry 'Point(1 1)@2000-01-03');
SELECT eIntersects(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeometry '{Point(2 2)@2000-01-01
, Point(2 2)@2000-01-03}');
SELECT eIntersects(tgeometry '{[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02], [Point(1 1)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '{Point(2 2)@2000-01-01, Point(2 2)@2000-01-02, Point(2 2)@2000-01-04, Point(2 2)@2000-01-06}');
SELECT eIntersects(tgeometry '{[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02], [Point(3 3)@2000-01-03, Point(3 3)@2000-01-04)}', tgeometry '[Point(2 2)@2000-01-01, Point(2 2)@2000-01-04]');
SELECT eIntersects(tgeometry '[Point(1 1)@2019-09-01, Point(2 2)@2019-09-02]', tgeometry '[Point(2 2)@2019-09-01, Point(1 1)@2019-09-02]');

/* Errors */
SELECT eIntersects(geometry 'SRID=3812;Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT eIntersects(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=3812;Point(1 1)');
SELECT eIntersects(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'SRID=3812;Point(1 1)@2000-01-01');

SELECT eIntersects(geography 'SRID=4283;Point(1 1)', tgeography 'Point(1 1)@2000-01-01');
SELECT eIntersects(tgeography 'Point(1 1)@2000-01-01', geography 'SRID=4283;Point(1 1)');
SELECT eIntersects(tgeography 'Point(1 1)@2000-01-01', tgeography 'SRID=4283;Point(1 1)@2000-01-01');

-------------------------------------------------------------------------------
-- eTouches
-------------------------------------------------------------------------------
-- The function does not support 3D or geographies

SELECT eTouches(geometry 'Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT eTouches(geometry 'Point(1 1)', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eTouches(geometry 'Point(1 1)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eTouches(geometry 'Point(1 1)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT eTouches(geometry 'Point empty', tgeometry 'Point(1 1)@2000-01-01');
SELECT eTouches(geometry 'Point empty', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eTouches(geometry 'Point empty', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eTouches(geometry 'Point empty', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT eTouches(tgeometry 'Point(1 1)@2000-01-01',  geometry 'Point(1 1)');
SELECT eTouches(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point(1 1)');
SELECT eTouches(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point(1 1)');
SELECT eTouches(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');

SELECT eTouches(tgeometry 'Point(1 1)@2000-01-01',  geometry 'Point empty');
SELECT eTouches(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point empty');
SELECT eTouches(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point empty');
SELECT eTouches(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point empty');

SELECT eTouches(geometry 'Point(1 1)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT eTouches(geometry 'Point(1 1)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT eTouches(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');

-- Coverage: Other geometry types
SELECT eTouches(tgeometry '[Point(0 0)@2000-01-01, Point(3 3)@2000-01-04]', geometry 'Triangle((1 1,1 2,2 1,1 1))');
SELECT eTouches(tgeometry '[Point(0 0)@2000-01-01, Point(3 3)@2000-01-04]', geometry 'CurvePolygon((1 1,2 2,3 1,2 0,1 1))');
-- Notice that the boundary of a closed line or circular string is empty !
SELECT eTouches(tgeometry '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]', geometry 'CircularString(1 1,2 2,3 1,2 0,1 1)');

/* Errors */
SELECT eTouches(geometry 'SRID=3812;Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT eTouches(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=3812;Point(1 1)');
SELECT eTouches(geometry 'Point(1 1 1)', tgeometry 'Point(1 1 1)@2000-01-01');

-------------------------------------------------------------------------------
-- eDwithin
-------------------------------------------------------------------------------

SELECT eDwithin(geometry 'Linestring(1 1,2 2)', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT eDwithin(geometry 'Linestring(1 1,2 2)', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT eDwithin(geometry 'Linestring(1 1,2 2)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT eDwithin(geometry 'Linestring(1 1,2 2)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT eDwithin(geometry 'Linestring empty', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT eDwithin(geometry 'Linestring empty', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT eDwithin(geometry 'Linestring empty', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT eDwithin(geometry 'Linestring empty', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT eDwithin(geometry 'Linestring(1 1 1,2 2 2)', tgeometry 'Point(1 1 1)@2000-01-01', 2);
SELECT eDwithin(geometry 'Linestring(1 1 1,2 2 2)', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT eDwithin(geometry 'Linestring(1 1 1,2 2 2)', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT eDwithin(geometry 'Linestring(1 1 1,2 2 2)', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT eDwithin(geometry 'Linestring Z empty', tgeometry 'Point(1 1 1)@2000-01-01', 2);
SELECT eDwithin(geometry 'Linestring Z empty', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT eDwithin(geometry 'Linestring Z empty', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT eDwithin(geometry 'Linestring Z empty', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT eDwithin(geography 'Linestring(1.5 1.5,2.5 2.5)', tgeography 'Point(1.5 1.5)@2000-01-01', 2);
SELECT eDwithin(geography 'Linestring(1.5 1.5,2.5 2.5)', tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT eDwithin(geography 'Linestring(1.5 1.5,2.5 2.5)', tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT eDwithin(geography 'Linestring(1.5 1.5,2.5 2.5)', tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);

SELECT eDwithin(geography 'Linestring empty', tgeography 'Point(1.5 1.5)@2000-01-01', 2);
SELECT eDwithin(geography 'Linestring empty', tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT eDwithin(geography 'Linestring empty', tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT eDwithin(geography 'Linestring empty', tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);

SELECT eDwithin(geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', tgeography 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT eDwithin(geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT eDwithin(geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT eDwithin(geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);

SELECT eDwithin(geography 'Linestring Z empty', tgeography 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT eDwithin(geography 'Linestring Z empty', tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT eDwithin(geography 'Linestring Z empty', tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT eDwithin(geography 'Linestring Z empty', tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);

SELECT eDwithin(tgeometry 'Point(1 1)@2000-01-01',  geometry 'Linestring(1 1,2 2)', 2);
SELECT eDwithin(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Linestring(1 1,2 2)', 2);
SELECT eDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Linestring(1 1,2 2)', 2);
SELECT eDwithin(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)', 2);

SELECT eDwithin(tgeometry 'Point(1 1)@2000-01-01',  geometry 'Linestring empty', 2);
SELECT eDwithin(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Linestring empty', 2);
SELECT eDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Linestring empty', 2);
SELECT eDwithin(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Linestring empty', 2);

SELECT eDwithin(tgeometry 'Point(1 1 1)@2000-01-01',  geometry 'Linestring(1 1 1,2 2 2)', 2);
SELECT eDwithin(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Linestring(1 1 1,2 2 2)', 2);
SELECT eDwithin(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Linestring(1 1 1,2 2 2)', 2);
SELECT eDwithin(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Linestring(1 1 1,2 2 2)', 2);

SELECT eDwithin(tgeometry 'Point(1 1 1)@2000-01-01',  geometry 'Linestring Z empty', 2);
SELECT eDwithin(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Linestring Z empty', 2);
SELECT eDwithin(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Linestring Z empty', 2);
SELECT eDwithin(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Linestring Z empty', 2);

SELECT eDwithin(tgeography 'Point(1.5 1.5)@2000-01-01',  geography 'Linestring(1.5 1.5,2.5 2.5)', 2);
SELECT eDwithin(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}',  geography 'Linestring(1.5 1.5,2.5 2.5)', 2);
SELECT eDwithin(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]',  geography 'Linestring(1.5 1.5,2.5 2.5)', 2);
SELECT eDwithin(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}',  geography 'Linestring(1.5 1.5,2.5 2.5)', 2);

SELECT eDwithin(tgeography 'Point(1.5 1.5)@2000-01-01',  geography 'Linestring empty', 2);
SELECT eDwithin(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}',  geography 'Linestring empty', 2);
SELECT eDwithin(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]',  geography 'Linestring empty', 2);
SELECT eDwithin(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}',  geography 'Linestring empty', 2);

SELECT eDwithin(tgeography 'Point(1.5 1.5 1.5)@2000-01-01',  geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', 2);
SELECT eDwithin(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}',  geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', 2);
SELECT eDwithin(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]',  geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', 2);
SELECT eDwithin(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}',  geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', 2);

SELECT eDwithin(tgeography 'Point(1.5 1.5 1.5)@2000-01-01',  geography 'Linestring Z empty', 2);
SELECT eDwithin(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}',  geography 'Linestring Z empty', 2);
SELECT eDwithin(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]',  geography 'Linestring Z empty', 2);
SELECT eDwithin(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}',  geography 'Linestring Z empty', 2);

-- NULL
SELECT eDwithin(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-02', 2);

SELECT eDwithin(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT eDwithin(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT eDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT eDwithin(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT eDwithin(tgeometry 'Point(1 1)@2000-01-01', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT eDwithin(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT eDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT eDwithin(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT eDwithin(tgeometry 'Point(1 1)@2000-01-01', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT eDwithin(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT eDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT eDwithin(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT eDwithin(tgeometry 'Point(1 1)@2000-01-01', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT eDwithin(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT eDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT eDwithin(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT eDwithin(tgeometry '[Point(1 1)@2000-01-02]', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-03}', 10);
SELECT eDwithin(tgeometry '[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03, Point(1 1)@2000-01-05]', tgeometry '{Point(1 1)@2000-01-04, Point(2 2)@2000-01-06}', 10);
SELECT eDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02]', tgeometry '[Point(2 2)@2000-01-01, Point(2 2)@2000-01-02]', 2);
SELECT eDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(0 0)@2000-01-02]', tgeometry '[Point(0 2)@2000-01-01, Point(1 1)@2000-01-02]', 2);
SELECT eDwithin(tgeometry '{[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(1 1)@2000-01-05]}', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-04}', 10);
SELECT eDwithin(tgeometry '{[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(1 1)@2000-01-06]}', tgeometry '[Point(1 1)@2000-01-04, Point(2 2)@2000-01-05]', 10);
SELECT eDwithin(tgeometry '{[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(1 1)@2000-01-06]}', tgeometry '{[Point(1 1)@2000-01-01],[Point(1 1)@2000-01-04, Point(2 2)@2000-01-05]}', 10);

SELECT eDwithin(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry 'Point(1 1 1)@2000-01-01', 2);
SELECT eDwithin(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry 'Point(1 1 1)@2000-01-01', 2);
SELECT eDwithin(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry 'Point(1 1 1)@2000-01-01', 2);
SELECT eDwithin(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry 'Point(1 1 1)@2000-01-01', 2);
SELECT eDwithin(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT eDwithin(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT eDwithin(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT eDwithin(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT eDwithin(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT eDwithin(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT eDwithin(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT eDwithin(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT eDwithin(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT eDwithin(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT eDwithin(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT eDwithin(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT eDwithin(tgeography 'Point(1.5 1.5)@2000-01-01', tgeography 'Point(1.5 1.5)@2000-01-01', 2);
SELECT eDwithin(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeography 'Point(1.5 1.5)@2000-01-01', 2);
SELECT eDwithin(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeography 'Point(1.5 1.5)@2000-01-01', 2);
SELECT eDwithin(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeography 'Point(1.5 1.5)@2000-01-01', 2);
SELECT eDwithin(tgeography 'Point(1.5 1.5)@2000-01-01', tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT eDwithin(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT eDwithin(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT eDwithin(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT eDwithin(tgeography 'Point(1.5 1.5)@2000-01-01', tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT eDwithin(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT eDwithin(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT eDwithin(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT eDwithin(tgeography 'Point(1.5 1.5)@2000-01-01', tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);
SELECT eDwithin(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);
SELECT eDwithin(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);
SELECT eDwithin(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);

SELECT eDwithin(tgeography 'Point(1.5 1.5 1.5)@2000-01-01', tgeography 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT eDwithin(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeography 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT eDwithin(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeography 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT eDwithin(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeography 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT eDwithin(tgeography 'Point(1.5 1.5 1.5)@2000-01-01', tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT eDwithin(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT eDwithin(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT eDwithin(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT eDwithin(tgeography 'Point(1.5 1.5 1.5)@2000-01-01', tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT eDwithin(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT eDwithin(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT eDwithin(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT eDwithin(tgeography 'Point(1.5 1.5 1.5)@2000-01-01', tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);
SELECT eDwithin(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);
SELECT eDwithin(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);
SELECT eDwithin(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);

SELECT eDwithin(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]}', tgeometry '{[Point(1 2)@2000-01-01, Point(2 3)@2000-01-02]}', 0.5);

-- Step interpolation
SELECT eDwithin(tgeometry 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

-- Mixed 2D/3D
SELECT eDwithin(geometry 'Point(1 1 1)', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT eDwithin(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1)', 2);
SELECT eDwithin(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-01', 2);

SELECT eDwithin(geography 'Point(1.5 1.5 1.5)', tgeography 'Point(1 1)@2000-01-01', 2);
SELECT eDwithin(tgeography 'Point(1 1 1)@2000-01-01', geography 'Point(1.5 1.5)', 2);
SELECT eDwithin(tgeography 'Point(1 1 1)@2000-01-01', tgeography 'Point(1 1)@2000-01-01', 2);

-- Coverage
SELECT eDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeometry '[Point(4 4)@2000-01-01, Point(2 1)@2000-01-02]', 1);
SELECT eDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeometry '[Point(4 4)@2000-01-01, Point(2 3)@2000-01-02]', 1);

/* Errors */
SELECT eDwithin(geometry 'SRID=3812;Point(1 1)', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT eDwithin(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=3812;Point(1 1)', 2);
SELECT eDwithin(tgeometry 'SRID=3812;Point(1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-01', 2);

SELECT eDwithin(geography 'SRID=4283;Point(1 1)', tgeography 'Point(1 1)@2000-01-01', 2);
SELECT eDwithin(tgeography 'Point(1 1)@2000-01-01', geography 'SRID=4283;Point(1 1)', 2);
SELECT eDwithin(tgeography 'SRID=4283;Point(1 1)@2000-01-01', tgeography 'Point(1 1)@2000-01-01', 2);

-------------------------------------------------------------------------------
