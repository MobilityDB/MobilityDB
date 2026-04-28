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
-- tContains
-------------------------------------------------------------------------------

SELECT tContains(geometry 'Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT tContains(geometry 'Point(1 1)', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tContains(geometry 'Point(1 1)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tContains(geometry 'Point(1 1)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tContains(geometry 'Point empty', tgeometry 'Point(1 1)@2000-01-01');
SELECT tContains(geometry 'Point empty', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tContains(geometry 'Point empty', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tContains(geometry 'Point empty', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tContains(geometry 'Linestring(1 1,2 2)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]');

/* Errors */
SELECT tContains(geometry 'SRID=5676;Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT tContains(geometry 'Point(1 1)', tgeometry 'SRID=5676;Point(1 1)@2000-01-01');
SELECT tContains(geometry 'Point(1 1)', tgeometry 'Point(1 1 1)@2000-01-01');
SELECT tContains(geometry 'Point(1 1 1)', tgeometry 'Point(1 1)@2000-01-01');

-------------------------------------------------------------------------------
-- tCovers
-------------------------------------------------------------------------------

SELECT tCovers(geometry 'Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT tCovers(geometry 'Point(1 1)', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tCovers(geometry 'Point(1 1)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tCovers(geometry 'Point(1 1)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

-- Empty geometry
SELECT tCovers(geometry 'Point empty', tgeometry 'Point(1 1)@2000-01-01');
SELECT tCovers(geometry 'Point empty', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tCovers(geometry 'Point empty', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tCovers(geometry 'Point empty', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tCovers(geometry 'Linestring(1 1,2 2)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]');

/* Errors */
SELECT tCovers(geometry 'SRID=5676;Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT tCovers(geometry 'Point(1 1)', tgeometry 'SRID=5676;Point(1 1)@2000-01-01');
SELECT tCovers(geometry 'Point(1 1)', tgeometry 'Point(1 1 1)@2000-01-01');
SELECT tCovers(geometry 'Point(1 1 1)', tgeometry 'Point(1 1)@2000-01-01');

-------------------------------------------------------------------------------

SELECT tCovers(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point(1 1)');
SELECT tCovers(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)');
SELECT tCovers(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)');
SELECT tCovers(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)');

-- Empty geometry
SELECT tCovers(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point empty');
SELECT tCovers(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point empty');
SELECT tCovers(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point empty');
SELECT tCovers(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point empty');

/* Errors */
SELECT tCovers(tgeometry 'SRID=5676;Point(1 1)@2000-01-01', geometry 'Point(1 1)');
SELECT tCovers(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT tCovers(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1)');
SELECT tCovers(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point(1 1 1)');

-------------------------------------------------------------------------------

SELECT tCovers(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-01');
SELECT tCovers(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry 'Point(1 1)@2000-01-01');
SELECT tCovers(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry 'Point(1 1)@2000-01-01');
SELECT tCovers(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry 'Point(1 1)@2000-01-01');

/* Errors */
SELECT tCovers(tgeometry 'SRID=5676;Point(1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-01');
SELECT tCovers(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'SRID=5676;Point(1 1)@2000-01-01');
SELECT tCovers(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-01');
SELECT tCovers(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(1 1 1)@2000-01-01');

-------------------------------------------------------------------------------
-- tDisjoint
-------------------------------------------------------------------------------

SELECT tDisjoint(geometry 'Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT tDisjoint(geometry 'Point(1 1)', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tDisjoint(geometry 'Point(1 1)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tDisjoint(geometry 'Point(1 1)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tDisjoint(geometry 'Point empty', tgeometry 'Point(1 1)@2000-01-01');
SELECT tDisjoint(geometry 'Point empty', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tDisjoint(geometry 'Point empty', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tDisjoint(geometry 'Point empty', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tDisjoint(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point(1 1)');
SELECT tDisjoint(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)');
SELECT tDisjoint(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)');
SELECT tDisjoint(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)');

SELECT tDisjoint(tgeometry '[Point(0 1)@2000-01-01, Point(2 1)@2000-01-04]', geometry 'Linestring(1 0,1 1,2 1,2 0)');
SELECT tDisjoint(tgeometry '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-04]', geometry 'Linestring(1 1,2 1)');
SELECT tDisjoint(tgeometry '[Point(1 1)@2000-01-01, Point(0 0)@2000-01-04]', geometry 'Linestring(0 0,1 1)');

SELECT tDisjoint(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point empty');
SELECT tDisjoint(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point empty');
SELECT tDisjoint(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point empty');
SELECT tDisjoint(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point empty');

--3D
SELECT tDisjoint(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Point(2 2 2)');
SELECT tDisjoint(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Point(2 2 2)');
SELECT tDisjoint(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Point(2 2 2)');
SELECT tDisjoint(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Point(2 2 2)');

/* Errors */
SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT tDisjoint(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');

-------------------------------------------------------------------------------
-- tIntersects
-------------------------------------------------------------------------------

SELECT tIntersects(geometry 'Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT tIntersects(geometry 'Point(1 1)', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tIntersects(geometry 'Point(1 1)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tIntersects(geometry 'Point(1 1)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tIntersects(geometry 'Point empty', tgeometry 'Point(1 1)@2000-01-01');
SELECT tIntersects(geometry 'Point empty', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tIntersects(geometry 'Point empty', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tIntersects(geometry 'Point empty', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tIntersects(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point(1 1)');
SELECT tIntersects(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)');
SELECT tIntersects(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)');
SELECT tIntersects(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)');

SELECT tIntersects(tgeometry '[Point(0 1)@2000-01-01, Point(2 1)@2000-01-04]', geometry 'Linestring(1 0,1 1,2 1,2 0)');
SELECT tIntersects(tgeometry '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-04]', geometry 'Linestring(1 1,2 1)');
SELECT tIntersects(tgeometry '[Point(1 1)@2000-01-01, Point(0 0)@2000-01-04]', geometry 'Linestring(0 0,1 1)');

SELECT tIntersects(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point empty');
SELECT tIntersects(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point empty');
SELECT tIntersects(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point empty');
SELECT tIntersects(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point empty');

SELECT tIntersects(tgeometry '[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02]', geometry 'Linestring(1 1,2 2)');
SELECT tIntersects(tgeometry '[Point(1 1)@2000-01-01, Point(4 1)@2000-01-02]', geometry 'Linestring(1 2,1 0,2 0,2 2)');

--3D
SELECT tIntersects(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Point(2 2 2)');
SELECT tIntersects(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Point(2 2 2)');
SELECT tIntersects(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Point(2 2 2)');
SELECT tIntersects(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Point(2 2 2)');

-- Coverage
SELECT tIntersects(tgeometry '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-03}', tgeometry 'Point(2 2)@2000-01-02');
SELECT tIntersects(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeometry '[Point(2 1)@2000-01-01, Point(1 2)@2000-01-02]');

/* Errors */
SELECT tIntersects(geometry 'SRID=5676;Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT tIntersects(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');

-------------------------------------------------------------------------------
-- tTouches
-------------------------------------------------------------------------------
-- The function does not support 3D or geographies

SELECT tTouches(geometry 'Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT tTouches(geometry 'Point(1 1)', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tTouches(geometry 'Point(1 1)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tTouches(geometry 'Point(1 1)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tTouches(geometry 'Point empty', tgeometry 'Point(1 1)@2000-01-01');
SELECT tTouches(geometry 'Point empty', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tTouches(geometry 'Point empty', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tTouches(geometry 'Point empty', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tTouches(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point(1 1)');
SELECT tTouches(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)');
SELECT tTouches(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)');
SELECT tTouches(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)');

SELECT tTouches(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point empty');
SELECT tTouches(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point empty');
SELECT tTouches(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point empty');
SELECT tTouches(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point empty');

SELECT tTouches(geometry 'Linestring(1 1,2 2)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]');

/* Errors */
SELECT tTouches(geometry 'SRID=5676;Point(1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT tTouches(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT tTouches(geometry 'Point(1 1 1)', tgeometry 'Point(1 1)@2000-01-01');
SELECT tTouches(geometry 'Point(1 1)', tgeometry 'Point(1 1 1)@2000-01-01');

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

SELECT tDwithin(geometry 'Point(1 1)', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT tDwithin(geometry 'Point(1 1)', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT tDwithin(geometry 'Point(1 1)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT tDwithin(geometry 'Point(1 1)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT tDwithin(geometry 'Point empty', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT tDwithin(geometry 'Point empty', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT tDwithin(geometry 'Point empty', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT tDwithin(geometry 'Point empty', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT tDwithin(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point(1 1)', 2);
SELECT tDwithin(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)', 2);
SELECT tDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)', 2);
SELECT tDwithin(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)', 2);

SELECT tDwithin(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point empty', 2);
SELECT tDwithin(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point empty', 2);
SELECT tDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point empty', 2);
SELECT tDwithin(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point empty', 2);

--3D
SELECT tDwithin(geometry 'Point(1 1 1)', tgeometry 'Point(1 1 1)@2000-01-01', 2);
SELECT tDwithin(geometry 'Point(1 1 1)', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT tDwithin(geometry 'Point(1 1 1)', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT tDwithin(geometry 'Point(1 1 1)', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT tDwithin(geometry 'Point Z empty', tgeometry 'Point(1 1 1)@2000-01-01', 2);
SELECT tDwithin(geometry 'Point Z empty', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT tDwithin(geometry 'Point Z empty', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT tDwithin(geometry 'Point Z empty', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT tDwithin(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1 1)', 2);
SELECT tDwithin(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Point(1 1 1)', 2);
SELECT tDwithin(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Point(1 1 1)', 2);
SELECT tDwithin(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Point(1 1 1)', 2);

SELECT tDwithin(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Point Z empty', 2);
SELECT tDwithin(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Point Z empty', 2);
SELECT tDwithin(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Point Z empty', 2);
SELECT tDwithin(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Point Z empty', 2);

-- Coverage
SELECT tDwithin(tgeometry '(Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', geometry 'Point(0 1)', 1);
SELECT tDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', geometry 'Point(2 3)', 1);

SELECT tDwithin(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT tDwithin(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT tDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT tDwithin(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT tDwithin(tgeometry 'Point(1 1)@2000-01-01', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT tDwithin(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT tDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT tDwithin(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT tDwithin(tgeometry 'Point(1 1)@2000-01-01', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT tDwithin(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT tDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT tDwithin(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT tDwithin(tgeometry 'Point(1 1)@2000-01-01', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT tDwithin(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT tDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT tDwithin(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT tDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(1 3)@2000-01-03]', geometry 'Point(1 2)', 0);
SELECT tDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(1 2)@2000-01-03]', geometry 'Point(1 3)', 0);
SELECT tDwithin(tgeometry '(Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]', tgeometry '[Point(1 0)@2000-01-01, Point(2 0)@2000-01-02]', 1);
SELECT tDwithin(tgeometry '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]', tgeometry '[Point(1 0)@2000-01-01, Point(2 0)@2000-01-02]', 1);
SELECT tDwithin(tgeometry '[Point(0 1)@2000-01-01, Point(0 0)@2000-01-02]', tgeometry '[Point(2 0)@2000-01-01, Point(1 0)@2000-01-02]', 1);
SELECT tDwithin(tgeometry '[Point(1 1)@2000-01-01, Point(0 0)@2000-01-02]', tgeometry '[Point(2 0)@2000-01-01, Point(1 1)@2000-01-02]', 1);
SELECT tDwithin(tgeometry '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]', tgeometry '[Point(0 2)@2000-01-01, Point(1 3)@2000-01-02]', 1);
SELECT tDwithin(tgeometry '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]', tgeometry '[Point(4 0)@2000-01-01, Point(3 1)@2000-01-02]', 0);

SELECT tDwithin(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry 'Point(1 1 1)@2000-01-01', 2);
SELECT tDwithin(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry 'Point(1 1 1)@2000-01-01', 2);
SELECT tDwithin(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry 'Point(1 1 1)@2000-01-01', 2);
SELECT tDwithin(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry 'Point(1 1 1)@2000-01-01', 2);
SELECT tDwithin(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT tDwithin(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT tDwithin(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT tDwithin(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT tDwithin(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT tDwithin(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT tDwithin(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT tDwithin(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT tDwithin(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT tDwithin(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT tDwithin(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT tDwithin(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT tDwithin(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03, Point(3 3)@
2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]
,[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 1);

-- Mixed 2D/3D
SELECT tDwithin(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-01', 2);

SELECT tDwithin(geometry 'Linestring(1 1,2 2)', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT tDwithin(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1,2 2)', 2);

/* Errors */
SELECT tDwithin(geometry 'SRID=5676;Point(1 1)', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT tDwithin(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)', 2);
SELECT tDwithin(tgeometry 'SRID=5676;Point(1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-01', 2);
SELECT tDwithin(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point(0 0)', -1);
SELECT tDwithin(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Point(0 0 0)', -1);

-------------------------------------------------------------------------------
