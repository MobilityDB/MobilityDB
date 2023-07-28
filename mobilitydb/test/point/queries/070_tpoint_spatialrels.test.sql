-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
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
-- econtains
-------------------------------------------------------------------------------

SELECT econtains(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT econtains(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT econtains(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT econtains(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT econtains(geometry 'Linestring(1 1,3 3)', tgeompoint '[Point(4 2)@2000-01-01, Point(2 4)@2000-01-02]');
SELECT econtains(geometry 'Linestring(1 1,3 3,1 1)', tgeompoint '[Point(4 2)@2000-01-01, Point(2 4)@2000-01-02]');
SELECT econtains(geometry 'Polygon((1 1,1 3,3 3,3 1,1 1))', tgeompoint '[Point(0 1)@2000-01-01, Point(4 1)@2000-01-02]');
SELECT econtains(geometry 'Polygon((1 1,1 3,3 3,3 1,1 1))', tgeompoint '[Point(1 4)@2000-01-01, Point(4 1)@2000-01-02]');

SELECT econtains(geometry 'Point empty', tgeompoint 'Point(1 1)@2000-01-01');
SELECT econtains(geometry 'Point empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT econtains(geometry 'Point empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT econtains(geometry 'Point empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

/* Errors */
SELECT econtains(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT econtains(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT econtains(geometry 'Point(1 1)', tgeompoint 'Point(1 1 1)@2000-01-01');

-------------------------------------------------------------------------------
-- edisjoint
-------------------------------------------------------------------------------

SELECT edisjoint(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT edisjoint(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT edisjoint(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT edisjoint(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT edisjoint(geometry 'Point empty', tgeompoint 'Point(1 1)@2000-01-01');
SELECT edisjoint(geometry 'Point empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT edisjoint(geometry 'Point empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT edisjoint(geometry 'Point empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT edisjoint(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT edisjoint(geometry 'Point(1 1 1)', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT edisjoint(geometry 'Point(1 1 1)', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT edisjoint(geometry 'Point(1 1 1)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT edisjoint(geometry 'Point Z empty', tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT edisjoint(geometry 'Point Z empty', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT edisjoint(geometry 'Point Z empty', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT edisjoint(geometry 'Point Z empty', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT edisjoint(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Point(1 1)');
SELECT edisjoint(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point(1 1)');
SELECT edisjoint(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point(1 1)');
SELECT edisjoint(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');

SELECT edisjoint(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Point empty');
SELECT edisjoint(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point empty');
SELECT edisjoint(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point empty');
SELECT edisjoint(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point empty');

SELECT edisjoint(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Point(1 1 1)');
SELECT edisjoint(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point(1 1 1)');
SELECT edisjoint(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point(1 1 1)');
SELECT edisjoint(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point(1 1 1)');

SELECT edisjoint(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Point Z empty');
SELECT edisjoint(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point Z empty');
SELECT edisjoint(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point Z empty');
SELECT edisjoint(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point Z empty');

SELECT edisjoint(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT edisjoint(geometry 'Point(1 1 1)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT edisjoint(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');
SELECT edisjoint(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point(1 1 1)');

-- Mixed 2D/3D
SELECT edisjoint(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT edisjoint(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1)');

-- Coverage
SELECT edisjoint(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))', tgeompoint '{Point(0 0)@2000-01-01, Point(0 2)@2000-01-02}');
SELECT edisjoint(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))', tgeompoint '[Point(0 0)@2000-01-01, Point(0 2)@2000-01-02]');
SELECT edisjoint(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))', tgeompoint '{[Point(0 0)@2000-01-01, Point(0 2)@2000-01-02],[Point(0 2)@2000-01-03, Point(0 0)@2000-01-04]}');
SELECT edisjoint(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))', tgeompoint '{Point(3 3)@2000-01-01, Point(4 4)@2000-01-02}');
SELECT edisjoint(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02]');

/* Errors */
SELECT edisjoint(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT edisjoint(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');

-------------------------------------------------------------------------------
-- eintersects
-------------------------------------------------------------------------------

------------------------
-- Geo x Temporal
------------------------
-- 2D
SELECT eintersects(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT eintersects(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eintersects(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eintersects(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
-- Empty
SELECT eintersects(geometry 'Point empty', tgeompoint 'Point(1 1)@2000-01-01');
SELECT eintersects(geometry 'Point empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eintersects(geometry 'Point empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eintersects(geometry 'Point empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
-- 3D
SELECT eintersects(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT eintersects(geometry 'Point(1 1 1)', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT eintersects(geometry 'Point(1 1 1)', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT eintersects(geometry 'Point(1 1 1)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
-- Empty
SELECT eintersects(geometry 'Point Z empty', tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT eintersects(geometry 'Point Z empty', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT eintersects(geometry 'Point Z empty', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT eintersects(geometry 'Point Z empty', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

-- Geography
SELECT eintersects(geography 'Point(1.5 1.5)', tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT eintersects(geography 'Point(1.5 1.5)', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT eintersects(geography 'Point(1.5 1.5)', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT eintersects(geography 'Point(1.5 1.5)', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
-- Empty
SELECT eintersects(geography 'Point empty', tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT eintersects(geography 'Point empty', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT eintersects(geography 'Point empty', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT eintersects(geography 'Point empty', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
-- 3D
SELECT eintersects(geography 'Point(1.5 1.5 1.5)', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT eintersects(geography 'Point(1.5 1.5 1.5)', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT eintersects(geography 'Point(1.5 1.5 1.5)', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT eintersects(geography 'Point(1.5 1.5 1.5)', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');
-- Empty
SELECT eintersects(geography 'Point Z empty', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT eintersects(geography 'Point Z empty', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT eintersects(geography 'Point Z empty', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT eintersects(geography 'Point Z empty', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

------------------------
-- Temporal x Geo
------------------------
SELECT eintersects(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Point(1 1)');
SELECT eintersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point(1 1)');
SELECT eintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point(1 1)');
SELECT eintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');
-- Empty
SELECT eintersects(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Point empty');
SELECT eintersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point empty');
SELECT eintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point empty');
SELECT eintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point empty');
-- 3D
SELECT eintersects(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Point(1 1 1)');
SELECT eintersects(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point(1 1 1)');
SELECT eintersects(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point(1 1 1)');
SELECT eintersects(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point(1 1 1)');
-- Empty
SELECT eintersects(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Point Z empty');
SELECT eintersects(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point Z empty');
SELECT eintersects(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point Z empty');
SELECT eintersects(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point Z empty');

-- Geography
SELECT eintersects(tgeogpoint 'Point(1.5 1.5)@2000-01-01',  geography 'Point(1.5 1.5)');
SELECT eintersects(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}',  geography 'Point(1.5 1.5)');
SELECT eintersects(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]',  geography 'Point(1.5 1.5)');
SELECT eintersects(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}',  geography 'Point(1.5 1.5)');
-- Empty
SELECT eintersects(tgeogpoint 'Point(1.5 1.5)@2000-01-01',  geography 'Point empty');
SELECT eintersects(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}',  geography 'Point empty');
SELECT eintersects(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]',  geography 'Point empty');
SELECT eintersects(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}',  geography 'Point empty');
-- 3D
SELECT eintersects(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01',  geography 'Point(1.5 1.5 1.5)');
SELECT eintersects(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}',  geography 'Point(1.5 1.5 1.5)');
SELECT eintersects(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]',  geography 'Point(1.5 1.5 1.5)');
SELECT eintersects(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}',  geography 'Point(1.5 1.5 1.5)');
-- Empty
SELECT eintersects(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01',  geography 'Point Z empty');
SELECT eintersects(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}',  geography 'Point Z empty');
SELECT eintersects(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]',  geography 'Point Z empty');
SELECT eintersects(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}',  geography 'Point Z empty');

------------------------
-- Temporal x Temporal
------------------------
-- Temporal x Instant
SELECT eintersects(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01');
SELECT eintersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(1 1)@2000-01-01');
SELECT eintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(1 1)@2000-01-01');
SELECT eintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(1 1)@2000-01-01');
-- Temporal x Discrete Sequence
SELECT eintersects(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eintersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT eintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
-- Temporal x Continuous Sequence
SELECT eintersects(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eintersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT eintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
-- Temporal x SequenceSet
SELECT eintersects(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT eintersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT eintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT eintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

-- Mixed 2D/3D
SELECT eintersects(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT eintersects(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1)');

SELECT eintersects(geography 'Point(1.5 1.5 1.5)', tgeogpoint 'Point(1 1)@2000-01-01');
SELECT eintersects(tgeogpoint 'Point(1 1 1)@2000-01-01', geography 'Point(1.5 1.5)');

-- Coverage
SELECT eintersects(tgeompoint '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-03}', tgeompoint 'Point(1 1)@2000-01-02');
SELECT eintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(1 1)@2000-01-04, Point(2 2)@2000-01-05]}', tgeompoint 'Point(1 1)@2000-01-03');
SELECT eintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint '{Point(2 2)@2000-01-01
, Point(2 2)@2000-01-03}');
SELECT eintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02], [Point(1 1)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{Point(2 2)@2000-01-01, Point(2 2)@2000-01-02, Point(2 2)@2000-01-04, Point(2 2)@2000-01-06}');
SELECT eintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02], [Point(3 3)@2000-01-03, Point(3 3)@2000-01-04)}', tgeompoint '[Point(2 2)@2000-01-01, Point(2 2)@2000-01-04]');
SELECT eintersects(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT eintersects(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]', tgeompoint 'Interp=Step;[Point(1 0)@2000-01-01, Point(1 1)@2000-01-02]');

/* Errors */
SELECT eintersects(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT eintersects(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT eintersects(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'SRID=5676;Point(1 1)@2000-01-01');

SELECT eintersects(geography 'SRID=4283;Point(1 1)', tgeogpoint 'Point(1 1)@2000-01-01');
SELECT eintersects(tgeogpoint 'Point(1 1)@2000-01-01', geography 'SRID=4283;Point(1 1)');
SELECT eintersects(tgeogpoint 'Point(1 1)@2000-01-01', tgeogpoint 'SRID=4283;Point(1 1)@2000-01-01');

-------------------------------------------------------------------------------
-- etouches
-------------------------------------------------------------------------------

SELECT etouches(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT etouches(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT etouches(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT etouches(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT etouches(geometry 'Point empty', tgeompoint 'Point(1 1)@2000-01-01');
SELECT etouches(geometry 'Point empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT etouches(geometry 'Point empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT etouches(geometry 'Point empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT etouches(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT etouches(geometry 'Point(1 1 1)', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT etouches(geometry 'Point(1 1 1)', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT etouches(geometry 'Point(1 1 1)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT etouches(geometry 'Point Z empty', tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT etouches(geometry 'Point Z empty', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT etouches(geometry 'Point Z empty', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT etouches(geometry 'Point Z empty', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT etouches(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Point(1 1)');
SELECT etouches(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point(1 1)');
SELECT etouches(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point(1 1)');
SELECT etouches(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');

SELECT etouches(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Point empty');
SELECT etouches(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point empty');
SELECT etouches(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point empty');
SELECT etouches(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point empty');

SELECT etouches(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Point(1 1 1)');
SELECT etouches(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point(1 1 1)');
SELECT etouches(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point(1 1 1)');
SELECT etouches(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point(1 1 1)');

SELECT etouches(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Point Z empty');
SELECT etouches(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point Z empty');
SELECT etouches(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point Z empty');
SELECT etouches(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point Z empty');

SELECT etouches(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT etouches(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT etouches(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');
SELECT etouches(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point(1 1 1)');

-- Mixed 2D/3D
SELECT etouches(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT etouches(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1)');

-- Coverage: Other geometry types
SELECT etouches(tgeompoint '[Point(0 0)@2000-01-01, Point(3 3)@2000-01-04]', geometry 'Triangle((1 1,1 2,2 1,1 1))');
SELECT etouches(tgeompoint '[Point(0 0)@2000-01-01, Point(3 3)@2000-01-04]', geometry 'CurvePolygon((1 1,2 2,3 1,2 0,1 1))');
-- Notice that the boundary of a closed circular string is empty !
SELECT etouches(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]', geometry 'CircularString(1 1,2 2,3 1,2 0,1 1)');
SELECT etouches(tgeompoint '[Point(0 0 0)@2000-01-01,Point(1 1 1)@2000-01-02]',
  geometry 'TIN (((0 0 0,0 0 1,0 1 0,0 0 0)),((0 0 0,0 1 0,1 1 0,0 0 0)))');


/* Errors */
SELECT etouches(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT etouches(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
-- unsupported geometry type
SELECT etouches(tgeompoint '[Point(0 0 0)@2000-01-01,Point(1 1 1)@2000-01-02]',
  geometry 'POLYHEDRALSURFACE( ((0 0 0, 0 0 1, 0 1 1, 0 1 0, 0 0 0)),
  ((0 0 0, 0 1 0, 1 1 0, 1 0 0, 0 0 0)), ((0 0 0, 1 0 0, 1 0 1, 0 0 1, 0 0 0)),
  ((1 1 0, 1 1 1, 1 0 1, 1 0 0, 1 1 0)),
  ((0 1 0, 0 1 1, 1 1 1, 1 1 0, 0 1 0)), ((0 0 1, 1 0 1, 1 1 1, 0 1 1, 0 0 1)) )');

-------------------------------------------------------------------------------
-- edwithin
-------------------------------------------------------------------------------

SELECT edwithin(geometry 'Linestring(1 1,2 2)', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT edwithin(geometry 'Linestring(1 1,2 2)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT edwithin(geometry 'Linestring(1 1,2 2)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT edwithin(geometry 'Linestring(1 1,2 2)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT edwithin(geometry 'Linestring empty', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT edwithin(geometry 'Linestring empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT edwithin(geometry 'Linestring empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT edwithin(geometry 'Linestring empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT edwithin(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT edwithin(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT edwithin(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT edwithin(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT edwithin(geometry 'Linestring Z empty', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT edwithin(geometry 'Linestring Z empty', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT edwithin(geometry 'Linestring Z empty', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT edwithin(geometry 'Linestring Z empty', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT edwithin(geography 'Linestring(1.5 1.5,2.5 2.5)', tgeogpoint 'Point(1.5 1.5)@2000-01-01', 2);
SELECT edwithin(geography 'Linestring(1.5 1.5,2.5 2.5)', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT edwithin(geography 'Linestring(1.5 1.5,2.5 2.5)', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT edwithin(geography 'Linestring(1.5 1.5,2.5 2.5)', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);

SELECT edwithin(geography 'Linestring empty', tgeogpoint 'Point(1.5 1.5)@2000-01-01', 2);
SELECT edwithin(geography 'Linestring empty', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT edwithin(geography 'Linestring empty', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT edwithin(geography 'Linestring empty', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);

SELECT edwithin(geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT edwithin(geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT edwithin(geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT edwithin(geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);

SELECT edwithin(geography 'Linestring Z empty', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT edwithin(geography 'Linestring Z empty', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT edwithin(geography 'Linestring Z empty', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT edwithin(geography 'Linestring Z empty', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);

SELECT edwithin(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Linestring(1 1,2 2)', 2);
SELECT edwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Linestring(1 1,2 2)', 2);
SELECT edwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Linestring(1 1,2 2)', 2);
SELECT edwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)', 2);

SELECT edwithin(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Linestring empty', 2);
SELECT edwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Linestring empty', 2);
SELECT edwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Linestring empty', 2);
SELECT edwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Linestring empty', 2);

SELECT edwithin(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Linestring(1 1 1,2 2 2)', 2);
SELECT edwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Linestring(1 1 1,2 2 2)', 2);
SELECT edwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Linestring(1 1 1,2 2 2)', 2);
SELECT edwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Linestring(1 1 1,2 2 2)', 2);

SELECT edwithin(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Linestring Z empty', 2);
SELECT edwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Linestring Z empty', 2);
SELECT edwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Linestring Z empty', 2);
SELECT edwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Linestring Z empty', 2);

SELECT edwithin(tgeogpoint 'Point(1.5 1.5)@2000-01-01',  geography 'Linestring(1.5 1.5,2.5 2.5)', 2);
SELECT edwithin(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}',  geography 'Linestring(1.5 1.5,2.5 2.5)', 2);
SELECT edwithin(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]',  geography 'Linestring(1.5 1.5,2.5 2.5)', 2);
SELECT edwithin(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}',  geography 'Linestring(1.5 1.5,2.5 2.5)', 2);

SELECT edwithin(tgeogpoint 'Point(1.5 1.5)@2000-01-01',  geography 'Linestring empty', 2);
SELECT edwithin(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}',  geography 'Linestring empty', 2);
SELECT edwithin(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]',  geography 'Linestring empty', 2);
SELECT edwithin(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}',  geography 'Linestring empty', 2);

SELECT edwithin(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01',  geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', 2);
SELECT edwithin(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}',  geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', 2);
SELECT edwithin(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]',  geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', 2);
SELECT edwithin(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}',  geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', 2);

SELECT edwithin(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01',  geography 'Linestring Z empty', 2);
SELECT edwithin(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}',  geography 'Linestring Z empty', 2);
SELECT edwithin(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]',  geography 'Linestring Z empty', 2);
SELECT edwithin(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}',  geography 'Linestring Z empty', 2);

-- NULL
SELECT edwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-02', 2);

SELECT edwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT edwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT edwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT edwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT edwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT edwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT edwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT edwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT edwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT edwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT edwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT edwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT edwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT edwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT edwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT edwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT edwithin(tgeompoint '[Point(1 1)@2000-01-02]', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-03}', 10);
SELECT edwithin(tgeompoint '[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03, Point(1 1)@2000-01-05]', tgeompoint '{Point(1 1)@2000-01-04, Point(2 2)@2000-01-06}', 10);
SELECT edwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02]', tgeompoint '[Point(2 2)@2000-01-01, Point(2 2)@2000-01-02]', 2);
SELECT edwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(0 0)@2000-01-02]', tgeompoint '[Point(0 2)@2000-01-01, Point(1 1)@2000-01-02]', 2);
SELECT edwithin(tgeompoint '{[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(1 1)@2000-01-05]}', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-04}', 10);
SELECT edwithin(tgeompoint '{[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(1 1)@2000-01-06]}', tgeompoint '[Point(1 1)@2000-01-04, Point(2 2)@2000-01-05]', 10);
SELECT edwithin(tgeompoint '{[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(1 1)@2000-01-06]}', tgeompoint '{[Point(1 1)@2000-01-01],[Point(1 1)@2000-01-04, Point(2 2)@2000-01-05]}', 10);

SELECT edwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT edwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT edwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT edwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT edwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT edwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT edwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT edwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT edwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT edwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT edwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT edwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT edwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT edwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT edwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT edwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT edwithin(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint 'Point(1.5 1.5)@2000-01-01', 2);
SELECT edwithin(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint 'Point(1.5 1.5)@2000-01-01', 2);
SELECT edwithin(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint 'Point(1.5 1.5)@2000-01-01', 2);
SELECT edwithin(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(1.5 1.5)@2000-01-01', 2);
SELECT edwithin(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT edwithin(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT edwithin(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT edwithin(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT edwithin(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT edwithin(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT edwithin(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT edwithin(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT edwithin(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);
SELECT edwithin(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);
SELECT edwithin(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);
SELECT edwithin(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);

SELECT edwithin(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT edwithin(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT edwithin(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT edwithin(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT edwithin(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT edwithin(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT edwithin(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT edwithin(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT edwithin(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT edwithin(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT edwithin(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT edwithin(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT edwithin(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);
SELECT edwithin(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);
SELECT edwithin(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);
SELECT edwithin(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);

SELECT edwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]}', tgeompoint '{[Point(1 2)@2000-01-01, Point(2 3)@2000-01-02]}', 0.5);

-- Step interpolation
SELECT edwithin(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

-- Mixed 2D/3D
SELECT edwithin(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT edwithin(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1)', 2);
SELECT edwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', 2);

SELECT edwithin(geography 'Point(1.5 1.5 1.5)', tgeogpoint 'Point(1 1)@2000-01-01', 2);
SELECT edwithin(tgeogpoint 'Point(1 1 1)@2000-01-01', geography 'Point(1.5 1.5)', 2);
SELECT edwithin(tgeogpoint 'Point(1 1 1)@2000-01-01', tgeogpoint 'Point(1 1)@2000-01-01', 2);

-- Coverage
SELECT edwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint '[Point(4 4)@2000-01-01, Point(2 1)@2000-01-02]', 1);
SELECT edwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint '[Point(4 4)@2000-01-01, Point(2 3)@2000-01-02]', 1);
SELECT edwithin(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint 'Interp=Step;[Point(5 0)@2000-01-01, Point(3 2)@2000-01-02]', 1);
SELECT edwithin(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint 'Interp=Step;[Point(2 2)@2000-01-01, Point(2 2)@2000-01-02]', 1);

/* Errors */
SELECT edwithin(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT edwithin(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)', 2);
SELECT edwithin(tgeompoint 'SRID=5676;Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', 2);

SELECT edwithin(geography 'SRID=4283;Point(1 1)', tgeogpoint 'Point(1 1)@2000-01-01', 2);
SELECT edwithin(tgeogpoint 'Point(1 1)@2000-01-01', geography 'SRID=4283;Point(1 1)', 2);
SELECT edwithin(tgeogpoint 'SRID=4283;Point(1 1)@2000-01-01', tgeogpoint 'Point(1 1)@2000-01-01', 2);

-------------------------------------------------------------------------------
