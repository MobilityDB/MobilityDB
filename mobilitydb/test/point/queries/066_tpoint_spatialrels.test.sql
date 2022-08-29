-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2022, PostGIS contributors
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
-- contains
-------------------------------------------------------------------------------

SELECT contains(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT contains(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT contains(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT contains(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT contains(geometry 'Point empty', tgeompoint 'Point(1 1)@2000-01-01');
SELECT contains(geometry 'Point empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT contains(geometry 'Point empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT contains(geometry 'Point empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

/* Errors */
SELECT contains(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT contains(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT contains(geometry 'Point(1 1)', tgeompoint 'Point(1 1 1)@2000-01-01');

-------------------------------------------------------------------------------
-- disjoint
-------------------------------------------------------------------------------

SELECT disjoint(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT disjoint(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT disjoint(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT disjoint(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT disjoint(geometry 'Point empty', tgeompoint 'Point(1 1)@2000-01-01');
SELECT disjoint(geometry 'Point empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT disjoint(geometry 'Point empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT disjoint(geometry 'Point empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT disjoint(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT disjoint(geometry 'Point(1 1 1)', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT disjoint(geometry 'Point(1 1 1)', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT disjoint(geometry 'Point(1 1 1)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT disjoint(geometry 'Point Z empty', tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT disjoint(geometry 'Point Z empty', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT disjoint(geometry 'Point Z empty', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT disjoint(geometry 'Point Z empty', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT disjoint(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Point(1 1)');
SELECT disjoint(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point(1 1)');
SELECT disjoint(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point(1 1)');
SELECT disjoint(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');

SELECT disjoint(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Point empty');
SELECT disjoint(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point empty');
SELECT disjoint(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point empty');
SELECT disjoint(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point empty');

SELECT disjoint(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Point(1 1 1)');
SELECT disjoint(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point(1 1 1)');
SELECT disjoint(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point(1 1 1)');
SELECT disjoint(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point(1 1 1)');

SELECT disjoint(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Point Z empty');
SELECT disjoint(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point Z empty');
SELECT disjoint(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point Z empty');
SELECT disjoint(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point Z empty');

SELECT disjoint(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT disjoint(geometry 'Point(1 1 1)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT disjoint(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');
SELECT disjoint(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point(1 1 1)');

-- Mixed 2D/3D
SELECT disjoint(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT disjoint(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1)');

-- Coverage
SELECT disjoint(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))', tgeompoint '{Point(0 0)@2000-01-01, Point(0 2)@2000-01-02}');
SELECT disjoint(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))', tgeompoint '[Point(0 0)@2000-01-01, Point(0 2)@2000-01-02]');
SELECT disjoint(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))', tgeompoint '{[Point(0 0)@2000-01-01, Point(0 2)@2000-01-02],[Point(0 2)@2000-01-03, Point(0 0)@2000-01-04]}');
SELECT disjoint(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))', tgeompoint '{Point(3 3)@2000-01-01, Point(4 4)@2000-01-02}');
SELECT disjoint(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02]');

/* Errors */
SELECT disjoint(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT disjoint(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');

-------------------------------------------------------------------------------
-- intersects
-------------------------------------------------------------------------------

------------------------
-- Geo x Temporal
------------------------
-- 2D
SELECT intersects(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT intersects(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT intersects(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT intersects(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
-- Empty
SELECT intersects(geometry 'Point empty', tgeompoint 'Point(1 1)@2000-01-01');
SELECT intersects(geometry 'Point empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT intersects(geometry 'Point empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT intersects(geometry 'Point empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
-- 3D
SELECT intersects(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT intersects(geometry 'Point(1 1 1)', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT intersects(geometry 'Point(1 1 1)', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT intersects(geometry 'Point(1 1 1)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
-- Empty
SELECT intersects(geometry 'Point Z empty', tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT intersects(geometry 'Point Z empty', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT intersects(geometry 'Point Z empty', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT intersects(geometry 'Point Z empty', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

-- Geography
SELECT intersects(geography 'Point(1.5 1.5)', tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT intersects(geography 'Point(1.5 1.5)', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT intersects(geography 'Point(1.5 1.5)', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT intersects(geography 'Point(1.5 1.5)', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
-- Empty
SELECT intersects(geography 'Point empty', tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT intersects(geography 'Point empty', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT intersects(geography 'Point empty', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT intersects(geography 'Point empty', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
-- 3D
SELECT intersects(geography 'Point(1.5 1.5 1.5)', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT intersects(geography 'Point(1.5 1.5 1.5)', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT intersects(geography 'Point(1.5 1.5 1.5)', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT intersects(geography 'Point(1.5 1.5 1.5)', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');
-- Empty
SELECT intersects(geography 'Point Z empty', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT intersects(geography 'Point Z empty', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT intersects(geography 'Point Z empty', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT intersects(geography 'Point Z empty', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

------------------------
-- Temporal x Geo
------------------------
SELECT intersects(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Point(1 1)');
SELECT intersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point(1 1)');
SELECT intersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point(1 1)');
SELECT intersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');
-- Empty
SELECT intersects(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Point empty');
SELECT intersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point empty');
SELECT intersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point empty');
SELECT intersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point empty');
-- 3D
SELECT intersects(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Point(1 1 1)');
SELECT intersects(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point(1 1 1)');
SELECT intersects(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point(1 1 1)');
SELECT intersects(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point(1 1 1)');
-- Empty
SELECT intersects(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Point Z empty');
SELECT intersects(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point Z empty');
SELECT intersects(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point Z empty');
SELECT intersects(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point Z empty');

-- Geography
SELECT intersects(tgeogpoint 'Point(1.5 1.5)@2000-01-01',  geography 'Point(1.5 1.5)');
SELECT intersects(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}',  geography 'Point(1.5 1.5)');
SELECT intersects(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]',  geography 'Point(1.5 1.5)');
SELECT intersects(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}',  geography 'Point(1.5 1.5)');
-- Empty
SELECT intersects(tgeogpoint 'Point(1.5 1.5)@2000-01-01',  geography 'Point empty');
SELECT intersects(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}',  geography 'Point empty');
SELECT intersects(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]',  geography 'Point empty');
SELECT intersects(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}',  geography 'Point empty');
-- 3D
SELECT intersects(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01',  geography 'Point(1.5 1.5 1.5)');
SELECT intersects(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}',  geography 'Point(1.5 1.5 1.5)');
SELECT intersects(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]',  geography 'Point(1.5 1.5 1.5)');
SELECT intersects(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}',  geography 'Point(1.5 1.5 1.5)');
-- Empty
SELECT intersects(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01',  geography 'Point Z empty');
SELECT intersects(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}',  geography 'Point Z empty');
SELECT intersects(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]',  geography 'Point Z empty');
SELECT intersects(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}',  geography 'Point Z empty');

------------------------
-- Temporal x Temporal
------------------------
-- Temporal x Instant
SELECT intersects(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01');
SELECT intersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(1 1)@2000-01-01');
SELECT intersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(1 1)@2000-01-01');
SELECT intersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(1 1)@2000-01-01');
-- Temporal x Discrete Sequence
SELECT intersects(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT intersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT intersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT intersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
-- Temporal x Continuous Sequence
SELECT intersects(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT intersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT intersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT intersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
-- Temporal x SequenceSet
SELECT intersects(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT intersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT intersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT intersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

-- Mixed 2D/3D
SELECT intersects(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT intersects(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1)');

SELECT intersects(geography 'Point(1.5 1.5 1.5)', tgeogpoint 'Point(1 1)@2000-01-01');
SELECT intersects(tgeogpoint 'Point(1 1 1)@2000-01-01', geography 'Point(1.5 1.5)');

-- Coverage
SELECT intersects(tgeompoint '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-03}', tgeompoint 'Point(1 1)@2000-01-02');
SELECT intersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(1 1)@2000-01-04, Point(2 2)@2000-01-05]}', tgeompoint 'Point(1 1)@2000-01-03');
SELECT intersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint '{Point(2 2)@2000-01-01
, Point(2 2)@2000-01-03}');
SELECT intersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02], [Point(1 1)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{Point(2 2)@2000-01-01, Point(2 2)@2000-01-02, Point(2 2)@2000-01-04, Point(2 2)@2000-01-06}');
SELECT intersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02], [Point(3 3)@2000-01-03, Point(3 3)@2000-01-04)}', tgeompoint '[Point(2 2)@2000-01-01, Point(2 2)@2000-01-04]');
SELECT intersects(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT intersects(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]', tgeompoint 'Interp=Stepwise;[Point(1 0)@2000-01-01, Point(1 1)@2000-01-02]');

/* Errors */
SELECT intersects(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT intersects(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT intersects(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'SRID=5676;Point(1 1)@2000-01-01');

SELECT intersects(geography 'SRID=4283;Point(1 1)', tgeogpoint 'Point(1 1)@2000-01-01');
SELECT intersects(tgeogpoint 'Point(1 1)@2000-01-01', geography 'SRID=4283;Point(1 1)');
SELECT intersects(tgeogpoint 'Point(1 1)@2000-01-01', tgeogpoint 'SRID=4283;Point(1 1)@2000-01-01');

-------------------------------------------------------------------------------
-- touches
-------------------------------------------------------------------------------

SELECT touches(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT touches(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT touches(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT touches(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT touches(geometry 'Point empty', tgeompoint 'Point(1 1)@2000-01-01');
SELECT touches(geometry 'Point empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT touches(geometry 'Point empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT touches(geometry 'Point empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT touches(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT touches(geometry 'Point(1 1 1)', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT touches(geometry 'Point(1 1 1)', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT touches(geometry 'Point(1 1 1)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT touches(geometry 'Point Z empty', tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT touches(geometry 'Point Z empty', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT touches(geometry 'Point Z empty', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT touches(geometry 'Point Z empty', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT touches(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Point(1 1)');
SELECT touches(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point(1 1)');
SELECT touches(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point(1 1)');
SELECT touches(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');

SELECT touches(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Point empty');
SELECT touches(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Point empty');
SELECT touches(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Point empty');
SELECT touches(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point empty');

SELECT touches(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Point(1 1 1)');
SELECT touches(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point(1 1 1)');
SELECT touches(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point(1 1 1)');
SELECT touches(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point(1 1 1)');

SELECT touches(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Point Z empty');
SELECT touches(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Point Z empty');
SELECT touches(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Point Z empty');
SELECT touches(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point Z empty');

SELECT touches(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT touches(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT touches(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)');
SELECT touches(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Point(1 1 1)');

-- Mixed 2D/3D
SELECT touches(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT touches(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1)');

-- Coverage: Other geometry types
SELECT touches(tgeompoint '[Point(0 0)@2000-01-01, Point(3 3)@2000-01-04]', geometry 'Triangle((1 1,1 2,2 1,1 1))');
SELECT touches(tgeompoint '[Point(0 0)@2000-01-01, Point(3 3)@2000-01-04]', geometry 'CurvePolygon((1 1,2 2,3 1,2 0,1 1))');
-- Notice that the boundary of a closed circular string is empty !
SELECT touches(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]', geometry 'CircularString(1 1,2 2,3 1,2 0,1 1)');

/* Errors */
SELECT touches(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT touches(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');

-------------------------------------------------------------------------------
-- dwithin
-------------------------------------------------------------------------------

SELECT dwithin(geometry 'Linestring(1 1,2 2)', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT dwithin(geometry 'Linestring(1 1,2 2)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT dwithin(geometry 'Linestring(1 1,2 2)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT dwithin(geometry 'Linestring(1 1,2 2)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT dwithin(geometry 'Linestring empty', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT dwithin(geometry 'Linestring empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT dwithin(geometry 'Linestring empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT dwithin(geometry 'Linestring empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT dwithin(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT dwithin(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT dwithin(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT dwithin(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT dwithin(geometry 'Linestring Z empty', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT dwithin(geometry 'Linestring Z empty', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT dwithin(geometry 'Linestring Z empty', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT dwithin(geometry 'Linestring Z empty', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT dwithin(geography 'Linestring(1.5 1.5,2.5 2.5)', tgeogpoint 'Point(1.5 1.5)@2000-01-01', 2);
SELECT dwithin(geography 'Linestring(1.5 1.5,2.5 2.5)', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT dwithin(geography 'Linestring(1.5 1.5,2.5 2.5)', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT dwithin(geography 'Linestring(1.5 1.5,2.5 2.5)', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);

SELECT dwithin(geography 'Linestring empty', tgeogpoint 'Point(1.5 1.5)@2000-01-01', 2);
SELECT dwithin(geography 'Linestring empty', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT dwithin(geography 'Linestring empty', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT dwithin(geography 'Linestring empty', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);

SELECT dwithin(geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT dwithin(geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT dwithin(geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT dwithin(geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);

SELECT dwithin(geography 'Linestring Z empty', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT dwithin(geography 'Linestring Z empty', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT dwithin(geography 'Linestring Z empty', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT dwithin(geography 'Linestring Z empty', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);

SELECT dwithin(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Linestring(1 1,2 2)', 2);
SELECT dwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Linestring(1 1,2 2)', 2);
SELECT dwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Linestring(1 1,2 2)', 2);
SELECT dwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Point(1 1)', 2);

SELECT dwithin(tgeompoint 'Point(1 1)@2000-01-01',  geometry 'Linestring empty', 2);
SELECT dwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}',  geometry 'Linestring empty', 2);
SELECT dwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]',  geometry 'Linestring empty', 2);
SELECT dwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}',  geometry 'Linestring empty', 2);

SELECT dwithin(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Linestring(1 1 1,2 2 2)', 2);
SELECT dwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Linestring(1 1 1,2 2 2)', 2);
SELECT dwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Linestring(1 1 1,2 2 2)', 2);
SELECT dwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Linestring(1 1 1,2 2 2)', 2);

SELECT dwithin(tgeompoint 'Point(1 1 1)@2000-01-01',  geometry 'Linestring Z empty', 2);
SELECT dwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}',  geometry 'Linestring Z empty', 2);
SELECT dwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]',  geometry 'Linestring Z empty', 2);
SELECT dwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}',  geometry 'Linestring Z empty', 2);

SELECT dwithin(tgeogpoint 'Point(1.5 1.5)@2000-01-01',  geography 'Linestring(1.5 1.5,2.5 2.5)', 2);
SELECT dwithin(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}',  geography 'Linestring(1.5 1.5,2.5 2.5)', 2);
SELECT dwithin(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]',  geography 'Linestring(1.5 1.5,2.5 2.5)', 2);
SELECT dwithin(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}',  geography 'Linestring(1.5 1.5,2.5 2.5)', 2);

SELECT dwithin(tgeogpoint 'Point(1.5 1.5)@2000-01-01',  geography 'Linestring empty', 2);
SELECT dwithin(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}',  geography 'Linestring empty', 2);
SELECT dwithin(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]',  geography 'Linestring empty', 2);
SELECT dwithin(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}',  geography 'Linestring empty', 2);

SELECT dwithin(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01',  geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', 2);
SELECT dwithin(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}',  geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', 2);
SELECT dwithin(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]',  geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', 2);
SELECT dwithin(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}',  geography 'Linestring(1.5 1.5 1.5,2.5 2.5 2.5)', 2);

SELECT dwithin(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01',  geography 'Linestring Z empty', 2);
SELECT dwithin(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}',  geography 'Linestring Z empty', 2);
SELECT dwithin(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]',  geography 'Linestring Z empty', 2);
SELECT dwithin(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}',  geography 'Linestring Z empty', 2);

-- NULL
SELECT dwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-02', 2);

SELECT dwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT dwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT dwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT dwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT dwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT dwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT dwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT dwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT dwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT dwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT dwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT dwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT dwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT dwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT dwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT dwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT dwithin(tgeompoint '[Point(1 1)@2000-01-02]', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-03}', 10);
SELECT dwithin(tgeompoint '[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03, Point(1 1)@2000-01-05]', tgeompoint '{Point(1 1)@2000-01-04, Point(2 2)@2000-01-06}', 10);
SELECT dwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02]', tgeompoint '[Point(2 2)@2000-01-01, Point(2 2)@2000-01-02]', 2);
SELECT dwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(0 0)@2000-01-02]', tgeompoint '[Point(0 2)@2000-01-01, Point(1 1)@2000-01-02]', 2);
SELECT dwithin(tgeompoint '{[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(1 1)@2000-01-05]}', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-04}', 10);
SELECT dwithin(tgeompoint '{[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(1 1)@2000-01-06]}', tgeompoint '[Point(1 1)@2000-01-04, Point(2 2)@2000-01-05]', 10);
SELECT dwithin(tgeompoint '{[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(1 1)@2000-01-06]}', tgeompoint '{[Point(1 1)@2000-01-01],[Point(1 1)@2000-01-04, Point(2 2)@2000-01-05]}', 10);

SELECT dwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT dwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT dwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT dwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT dwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT dwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT dwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT dwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT dwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT dwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT dwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT dwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT dwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT dwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT dwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT dwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT dwithin(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint 'Point(1.5 1.5)@2000-01-01', 2);
SELECT dwithin(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint 'Point(1.5 1.5)@2000-01-01', 2);
SELECT dwithin(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint 'Point(1.5 1.5)@2000-01-01', 2);
SELECT dwithin(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(1.5 1.5)@2000-01-01', 2);
SELECT dwithin(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT dwithin(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT dwithin(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT dwithin(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 2);
SELECT dwithin(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT dwithin(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT dwithin(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT dwithin(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 2);
SELECT dwithin(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);
SELECT dwithin(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);
SELECT dwithin(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);
SELECT dwithin(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2);

SELECT dwithin(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT dwithin(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT dwithin(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT dwithin(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', 2);
SELECT dwithin(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT dwithin(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT dwithin(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT dwithin(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 2);
SELECT dwithin(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT dwithin(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT dwithin(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT dwithin(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 2);
SELECT dwithin(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);
SELECT dwithin(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);
SELECT dwithin(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);
SELECT dwithin(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2);

SELECT dwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]}', tgeompoint '{[Point(1 2)@2000-01-01, Point(2 3)@2000-01-02]}', 0.5);

-- Mixed 2D/3D
SELECT dwithin(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT dwithin(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1)', 2);
SELECT dwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', 2);

SELECT dwithin(geography 'Point(1.5 1.5 1.5)', tgeogpoint 'Point(1 1)@2000-01-01', 2);
SELECT dwithin(tgeogpoint 'Point(1 1 1)@2000-01-01', geography 'Point(1.5 1.5)', 2);
SELECT dwithin(tgeogpoint 'Point(1 1 1)@2000-01-01', tgeogpoint 'Point(1 1)@2000-01-01', 2);

-- Coverage
SELECT dwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint '[Point(4 4)@2000-01-01, Point(2 1)@2000-01-02]', 1);
SELECT dwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint '[Point(4 4)@2000-01-01, Point(2 3)@2000-01-02]', 1);
SELECT dwithin(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint 'Interp=Stepwise;[Point(5 0)@2000-01-01, Point(3 2)@2000-01-02]', 1);
SELECT dwithin(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint 'Interp=Stepwise;[Point(2 2)@2000-01-01, Point(2 2)@2000-01-02]', 1);

/* Errors */
SELECT dwithin(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT dwithin(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)', 2);
SELECT dwithin(tgeompoint 'SRID=5676;Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', 2);

SELECT dwithin(geography 'SRID=4283;Point(1 1)', tgeogpoint 'Point(1 1)@2000-01-01', 2);
SELECT dwithin(tgeogpoint 'Point(1 1)@2000-01-01', geography 'SRID=4283;Point(1 1)', 2);
SELECT dwithin(tgeogpoint 'SRID=4283;Point(1 1)@2000-01-01', tgeogpoint 'Point(1 1)@2000-01-01', 2);

-------------------------------------------------------------------------------
