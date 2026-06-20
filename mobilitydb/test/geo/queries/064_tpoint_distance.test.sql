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

-- NULL
SELECT round(tgeompoint '[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03)' <-> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(2 2)@2000-01-04}', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03)}' <-> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(2 2)@2000-01-04}', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02),(Point(1 1)@2000-01-03, Point(2 2)@2000-01-04]}' <-> tgeompoint '(Point(1 1)@2000-01-02, Point(2 2)@2000-01-03)', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02),(Point(1 1)@2000-01-03, Point(2 2)@2000-01-04]}' <-> tgeompoint '{(Point(1 1)@2000-01-02, Point(2 2)@2000-01-03)}', 6);
/* Errors */
SELECT round(geometry 'Linestring(1 1,2 2)' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(geometry 'srid=5676;Point(2 2)' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(geometry 'Point(2 2 2)' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> geometry 'Linestring(1 1,2 2)', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> geometry 'srid=5676;Point(2 2)', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> geometry 'Point(2 2 2)', 6);
SELECT round(tgeompoint 'SRID=5676;[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2000-01-01, Point(3 3 3)@2000-01-03]' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);


SELECT round(geometry 'Point(1 1)' <-> tgeompoint 'Point(2 2)@2000-01-01', 6);
SELECT round(geometry 'Point(1 1)' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(geometry 'Point(1 1)' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(geometry 'Point(2 2)' <-> tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(geometry 'Point(1 1)' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);

SELECT round(geometry 'Point empty' <-> tgeompoint 'Point(2 2)@2000-01-01', 6);
SELECT round(geometry 'Point empty' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(geometry 'Point empty' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(geometry 'Point empty' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);

SELECT round(geometry 'Point(1 1 1)' <-> tgeompoint 'Point(2 2 2)@2000-01-01', 6);
SELECT round(geometry 'Point(1 1 1)' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(geometry 'Point(1 1 1)' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(geometry 'Point(1 1 1)' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);

SELECT round(geometry 'Point Z empty' <-> tgeompoint 'Point(2 2 2)@2000-01-01', 6);
SELECT round(geometry 'Point Z empty' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(geometry 'Point Z empty' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(geometry 'Point Z empty' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);

SELECT round(geography 'Point(-90 0)' <-> tgeogpoint 'Point(0 -90)@2000-01-01', 6);
SELECT round(geography 'Point(-90 0)' <-> tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}', 6);
SELECT round(geography 'Point(-90 0)' <-> tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]', 6);
SELECT round(geography 'Point(-90 0)' <-> tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', 6);

SELECT round(geography 'Point empty' <-> tgeogpoint 'Point(0 -90)@2000-01-01', 6);
SELECT round(geography 'Point empty' <-> tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}', 6);
SELECT round(geography 'Point empty' <-> tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]', 6);
SELECT round(geography 'Point empty' <-> tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', 6);

SELECT round(geography 'Point(-90 0 100)' <-> tgeogpoint 'Point(0 -90 100)@2000-01-01', 6);
SELECT round(geography 'Point(-90 0 100)' <-> tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}', 6);
SELECT round(geography 'Point(-90 0 100)' <-> tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]', 6);
SELECT round(geography 'Point(-90 0 100)' <-> tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', 6);

SELECT round(geography 'Point Z empty' <-> tgeogpoint 'Point(0 -90 100)@2000-01-01', 6);
SELECT round(geography 'Point Z empty' <-> tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}', 6);
SELECT round(geography 'Point Z empty' <-> tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]', 6);
SELECT round(geography 'Point Z empty' <-> tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', 6);

SELECT round(tgeompoint 'Point(1 1)@2000-01-01' <-> geometry 'Point(1 1)', 6);
SELECT round(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> geometry 'Point(1 1)', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> geometry 'Point(1 1)', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> geometry 'Point(1 1)', 6);

SELECT round(tgeompoint 'Point(1 1)@2000-01-01' <-> geometry 'Point empty', 6);
SELECT round(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> geometry 'Point empty', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> geometry 'Point empty', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> geometry 'Point empty', 6);

SELECT round(tgeompoint 'Point(1 1 1)@2000-01-01' <-> geometry 'Point(1 1 1)', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> geometry 'Point(1 1 1)', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> geometry 'Point(1 1 1)', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> geometry 'Point(1 1 1)', 6);

SELECT round(tgeompoint 'Point(1 1 1)@2000-01-01' <-> geometry 'Point Z empty', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> geometry 'Point Z empty', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> geometry 'Point Z empty', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> geometry 'Point Z empty', 6);

SELECT round(tgeogpoint 'Point(-90 0)@2000-01-01' <-> geography 'Point(-90 0)', 6);
SELECT round(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' <-> geography 'Point(-90 0)', 6);
SELECT round(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' <-> geography 'Point(-90 0)', 6);
SELECT round(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' <-> geography 'Point(-90 0)', 6);

SELECT round(tgeogpoint 'Point(-90 0)@2000-01-01' <-> geography 'Point empty', 6);
SELECT round(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' <-> geography 'Point empty', 6);
SELECT round(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' <-> geography 'Point empty', 6);
SELECT round(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' <-> geography 'Point empty', 6);

SELECT round(tgeogpoint 'Point(-90 0 100)@2000-01-01' <-> geography 'Point(-90 0 100)', 6);
SELECT round(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' <-> geography 'Point(-90 0 100)', 6);
SELECT round(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' <-> geography 'Point(-90 0 100)', 6);
SELECT round(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' <-> geography 'Point(-90 0 100)', 6);

SELECT round(tgeogpoint 'Point(-90 0 100)@2000-01-01' <-> geography 'Point Z empty', 6);
SELECT round(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' <-> geography 'Point Z empty', 6);
SELECT round(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' <-> geography 'Point Z empty', 6);
SELECT round(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' <-> geography 'Point Z empty', 6);

SELECT round(tgeompoint 'Point(1 1)@2000-01-01' <-> tgeompoint 'Point(2 2)@2000-01-01', 6);
SELECT round(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeompoint 'Point(2 2)@2000-01-01', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeompoint 'Point(2 2)@2000-01-01', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeompoint 'Point(2 2)@2000-01-01', 6);
SELECT round(tgeompoint 'Point(1 1)@2000-01-01' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(tgeompoint 'Point(1 1)@2000-01-01' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(tgeompoint 'Point(1 1)@2000-01-01' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);
SELECT round(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);
SELECT round(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);
SELECT round(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);

SELECT round(tgeompoint 'Point(1 1 1)@2000-01-01' <-> tgeompoint 'Point(2 2 2)@2000-01-01', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeompoint 'Point(2 2 2)@2000-01-01', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeompoint 'Point(2 2 2)@2000-01-01', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeompoint 'Point(2 2 2)@2000-01-01', 6);
SELECT round(tgeompoint 'Point(1 1 1)@2000-01-01' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(tgeompoint 'Point(1 1 1)@2000-01-01' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(tgeompoint 'Point(1 1 1)@2000-01-01' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);
SELECT round(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);
SELECT round(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);
SELECT round(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);

SELECT round(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]' <-> tgeompoint 'Interp=Step;[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02]', 6);

SELECT round(tgeogpoint 'Point(-90 0)@2000-01-01' <-> tgeogpoint 'Point(0 -90)@2000-01-01', 6);
SELECT round(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' <-> tgeogpoint 'Point(0 -90)@2000-01-01', 6);
SELECT round(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' <-> tgeogpoint 'Point(0 -90)@2000-01-01', 6);
SELECT round(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' <-> tgeogpoint 'Point(0 -90)@2000-01-01', 6);
SELECT round(tgeogpoint 'Point(-90 0)@2000-01-01' <-> tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}', 6);
SELECT round(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' <-> tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}', 6);
SELECT round(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' <-> tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}', 6);
SELECT round(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' <-> tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}', 6);
SELECT round(tgeogpoint 'Point(-90 0)@2000-01-01' <-> tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]', 6);
SELECT round(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' <-> tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]', 6);
SELECT round(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' <-> tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]', 6);
SELECT round(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' <-> tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]', 6);
SELECT round(tgeogpoint 'Point(-90 0)@2000-01-01' <-> tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', 6);
SELECT round(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' <-> tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', 6);
SELECT round(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' <-> tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', 6);
SELECT round(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' <-> tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', 6);

SELECT round(tgeogpoint 'Point(-90 0 100)@2000-01-01' <-> tgeogpoint 'Point(0 -90 100)@2000-01-01', 6);
SELECT round(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' <-> tgeogpoint 'Point(0 -90 100)@2000-01-01', 6);
SELECT round(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' <-> tgeogpoint 'Point(0 -90 100)@2000-01-01', 6);
SELECT round(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' <-> tgeogpoint 'Point(0 -90 100)@2000-01-01', 6);
SELECT round(tgeogpoint 'Point(-90 0 100)@2000-01-01' <-> tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}', 6);
SELECT round(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' <-> tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}', 6);
SELECT round(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' <-> tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}', 6);
SELECT round(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' <-> tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}', 6);
SELECT round(tgeogpoint 'Point(-90 0 100)@2000-01-01' <-> tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]', 6);
SELECT round(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' <-> tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]', 6);
SELECT round(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' <-> tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]', 6);
SELECT round(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' <-> tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]', 6);
SELECT round(tgeogpoint 'Point(-90 0 100)@2000-01-01' <-> tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', 6);
SELECT round(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' <-> tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', 6);
SELECT round(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' <-> tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', 6);
SELECT round(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' <-> tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', 6);

-------------------------------------------------------------------------------

SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));

SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02]', geometry 'Linestring(1 1,3 3)'));

SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0)@2000-01-01', geography 'Linestring(90 0,0 90)'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', geography 'Linestring(90 0,0 90)'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', geography 'Linestring(90 0,0 90)'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', geography 'Linestring(90 0,0 90)'),6));
SELECT asText(NearestApproachInstant(tgeogpoint 'Interp=Step;[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', geography 'Linestring(90 0,0 90)'));
SELECT asText(NearestApproachInstant(tgeogpoint 'Interp=Step;{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', geography 'Linestring(90 0,0 90)'));
SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0)@2000-01-01', geography 'Linestring empty'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', geography 'Linestring empty'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', geography 'Linestring empty'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', geography 'Linestring empty'),6));
SELECT asText(NearestApproachInstant(tgeogpoint 'Interp=Step;[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', geography 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeogpoint 'Interp=Step;{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', geography 'Linestring empty'));

SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(geography 'Linestring(90 0,0 90)', tgeogpoint 'Point(-90 0)@2000-01-01'));
SELECT asText(NearestApproachInstant(geography 'Linestring(90 0,0 90)', tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}'));
SELECT asText(NearestApproachInstant(geography 'Linestring(90 0,0 90)', tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]'));
SELECT asText(NearestApproachInstant(geography 'Linestring(90 0,0 90)', tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeogpoint 'Point(-90 0)@2000-01-01'));
SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}'));
SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]'));
SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02)', tgeompoint '[Point(3 3)@2000-01-01, Point(2 2)@2000-01-02)'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02], (Point(1 1)@2000-01-03, Point(0 0)@2000-01-04]}', tgeompoint '[Point(3 3)@2000-01-01, Point(3 3)@2000-01-02, Point(2 2)@2000-01-03, Point(3 3)@2000-01-04)'));

SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));

SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0)@2000-01-01', tgeogpoint 'Point(0 -90)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeogpoint 'Point(0 -90)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeogpoint 'Point(0 -90)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeogpoint 'Point(0 -90)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0)@2000-01-01', tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0)@2000-01-01', tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-03]', tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0)@2000-01-01', tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-03]', tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'),6));

SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0 100)@2000-01-01', tgeogpoint 'Point(0 -90 100)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeogpoint 'Point(0 -90 100)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeogpoint 'Point(0 -90 100)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeogpoint 'Point(0 -90 100)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0 100)@2000-01-01', tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0 100)@2000-01-01', tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-03]', tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint 'Point(-90 0 100)@2000-01-01', tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-03]', tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'),6));

/* Errors */
SELECT NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');
SELECT NearestApproachInstant(geometry 'SRID=5676;Linestring(1 1,2 2)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT NearestApproachInstant(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'SRID=5676;Point(1 1)@2000-01-01');
SELECT NearestApproachInstant(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1 1)@2000-01-01');

--------------------------------------------------------

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'), 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'), 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'), 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Interp=Step;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty'), 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0)@2000-01-01', geography 'Linestring(90 0,0 90)'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', geography 'Linestring(90 0,0 90)'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', geography 'Linestring(90 0,0 90)'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', geography 'Linestring(90 0,0 90)'), 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0)@2000-01-01', geography 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', geography 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', geography 'Linestring empty'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', geography 'Linestring empty'), 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0 100)@2000-01-01', geography 'Linestring(90 0 0,0 90 100)'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', geography 'Linestring(90 0 0,0 90 100)'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', geography 'Linestring(90 0 0,0 90 100)'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', geography 'Linestring(90 0 0,0 90 100)'), 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0 100)@2000-01-01', geography 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', geography 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', geography 'Linestring Z empty'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', geography 'Linestring Z empty'), 6);

SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeompoint 'Point(1 1)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);

SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeompoint 'Point(1 1)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);

SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint 'Point(1 1 1)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);

SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeompoint 'Point(1 1 1)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);

SELECT round(NearestApproachDistance(geography 'Linestring(90 0,0 90)', tgeogpoint 'Point(-90 0)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0,0 90)', tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0,0 90)', tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0,0 90)', tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 6);

SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeogpoint 'Point(-90 0)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 6);

SELECT round(NearestApproachDistance(geography 'Linestring(90 0 0,0 90 100)', tgeogpoint 'Point(-90 0 100)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0 0,0 90 100)', tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0 0,0 90 100)', tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0 0,0 90 100)', tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'), 6);

SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeogpoint 'Point(-90 0 100)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'), 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(2 2)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(2 2)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(2 2)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(2 2)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);

SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint 'Point(2 2 2)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint 'Point(2 2 2)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint 'Point(2 2 2)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint 'Point(2 2 2)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0)@2000-01-01', tgeogpoint 'Point(0 -90)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeogpoint 'Point(0 -90)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeogpoint 'Point(0 -90)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeogpoint 'Point(0 -90)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0)@2000-01-01', tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0)@2000-01-01', tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0)@2000-01-01', tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 6);

SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0 100)@2000-01-01', tgeogpoint 'Point(0 -90 100)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeogpoint 'Point(0 -90 100)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeogpoint 'Point(0 -90 100)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeogpoint 'Point(0 -90 100)@2000-01-01'), 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0 100)@2000-01-01', tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0 100)@2000-01-01', tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'), 6);
SELECT round(NearestApproachDistance(tgeogpoint 'Point(-90 0 100)@2000-01-01', tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'), 6);
SELECT round(NearestApproachDistance(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'), 6);

SELECT round((stbox 'STBOX XT(((0,0),(1,1)),[2000-01-01,2000-01-02])' |=| stbox 'STBOX XT(((2,2),(2,3)),[2000-01-01,2000-01-02])'), 6);
SELECT round((stbox 'STBOX XT(((0,0),(1,1)),[2000-01-01,2000-01-02])' |=| stbox 'STBOX XT(((2,2),(3,3)),[2000-01-03,2000-01-04])'), 6);
SELECT round((stbox 'GEODSTBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' |=| stbox 'GEODSTBOX ZT(((2,2,2),(3,3,3)),[2000-01-01,2000-01-02])'), 6);
-- 3D
SELECT round((stbox 'STBOX ZT(((0,0,0),(1,1,1)),[2000-01-01,2000-01-02])' |=| stbox 'STBOX ZT(((2,2,2),(3,3,3)),[2000-01-01,2000-01-02])'), 6);

SELECT round((stbox 'STBOX XT(((1,1),(1,1)),[2000-01-01,2000-01-02])' |=| geometry 'Point empty'), 6);
SELECT round((stbox 'STBOX XT(((1,1),(1,1)),[2000-01-01,2000-01-02])' |=| geometry 'Point(0 0)'), 6);
SELECT round((stbox 'STBOX ZT(((1,1,1),(1,1,1)),[2000-01-01,2000-01-02])' |=| geometry 'Point (0 0 0)'), 6);

SELECT round((geometry 'Point empty' |=| stbox 'STBOX XT(((1,1),(1,1)),[2000-01-01,2000-01-02])'), 6);
SELECT round((geometry 'Point(0 0)' |=| stbox 'STBOX XT(((1,1),(1,1)),[2000-01-01,2000-01-02])'), 6);
SELECT round((geometry 'Point (0 0 0)' |=| stbox 'STBOX ZT(((1,1,1),(1,1,1)),[2000-01-01,2000-01-02])'), 6);

/* Errors */
SELECT stbox 'STBOX XT(((1,1),(1,1)),[2000-01-01,2000-01-02])' |=| geometry 'SRID=5676;Point(0 0)';
SELECT stbox 'STBOX XT(((1,1),(1,1)),[2000-01-01,2000-01-02])' |=| geometry 'Point(0 0 0)';
SELECT stbox 'STBOX X((1.0,2.0),(3.0,4.0))' |=| stbox 'STBOX Z((1.0,2.0,3.0),(4.0,5.0,6.0))';
SELECT stbox 'GEODSTBOX ZT(((1.0,2.0,3.0),(1.0,2.0,3.0)),[2001-01-03,2001-01-04])' |=| geography 'Point(0 0)';

SELECT round((tgeompoint 'Point(1 1)@2000-01-01' |=| stbox 'STBOX XT(((2,2),(2,2)),[2000-01-01,2000-01-02])'), 6);
SELECT round((tgeompoint 'Point(1 1)@2000-01-01' |=| stbox 'STBOX XT(((2,2),(2,2)),[2000-01-02,2000-01-03])'), 6);

/* Errors */
SELECT round((tgeompoint 'Point(1 1 1)@2000-01-01' |=| stbox 'STBOX XT(((2,2),(2,2)),[2000-01-01,2000-01-02])'), 6);
SELECT round((tgeogpoint 'Point(1 1)@2000-01-01' |=| stbox 'GEODSTBOX ZT(((2,2,2),(2,2,2)),[2000-01-01,2000-01-02])'), 6);

SELECT round((stbox 'STBOX XT(((2,2),(2,2)),[2000-01-01,2000-01-02])' |=| tgeompoint 'Point(1 1)@2000-01-01' ), 6);
SELECT round((stbox 'STBOX XT(((2,2),(2,2)),[2000-01-02,2000-01-03])' |=| tgeompoint 'Point(1 1)@2000-01-01' ), 6);

SELECT round((tgeompoint 'Point(1 1)@2000-01-01' |=| geometry 'Linestring(0 0,3 3)'), 6);
SELECT round((tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| geometry 'Linestring(0 0,3 3)'), 6);
SELECT round((tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| geometry 'Linestring(0 0,3 3)'), 6);
SELECT round((tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| geometry 'Linestring(0 0,3 3)'), 6);

SELECT round((tgeompoint 'Point(1 1)@2000-01-01' |=| geometry 'Linestring empty'), 6);
SELECT round((tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| geometry 'Linestring empty'), 6);
SELECT round((tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| geometry 'Linestring empty'), 6);
SELECT round((tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| geometry 'Linestring empty'), 6);

SELECT round((tgeompoint 'Point(1 1 1)@2000-01-01' |=| geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round((tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round((tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| geometry 'Linestring(0 0 0,3 3 3)'), 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| geometry 'Linestring(0 0 0,3 3 3)'), 6);

SELECT round((tgeompoint 'Point(1 1 1)@2000-01-01' |=| geometry 'Linestring Z empty'), 6);
SELECT round((tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| geometry 'Linestring Z empty'), 6);
SELECT round((tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| geometry 'Linestring Z empty'), 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| geometry 'Linestring Z empty'), 6);

SELECT round((tgeogpoint 'Point(-90 0)@2000-01-01' |=| geography 'Linestring(90 0,0 90)'), 6);
SELECT round((tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' |=| geography 'Linestring(90 0,0 90)'), 6);
SELECT round((tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' |=| geography 'Linestring(90 0,0 90)'), 6);
SELECT round((tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' |=| geography 'Linestring(90 0,0 90)'), 6);

SELECT round((tgeogpoint 'Point(-90 0)@2000-01-01' |=| geography 'Linestring empty'), 6);
SELECT round((tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' |=| geography 'Linestring empty'), 6);
SELECT round((tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' |=| geography 'Linestring empty'), 6);
SELECT round((tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' |=| geography 'Linestring empty'), 6);

SELECT round((tgeogpoint 'Point(-90 0 100)@2000-01-01' |=| geography 'Linestring(90 0 0,0 90 100)'), 6);
SELECT round((tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' |=| geography 'Linestring(90 0 0,0 90 100)'), 6);
SELECT round((tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' |=| geography 'Linestring(90 0 0,0 90 100)'), 6);
SELECT round((tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' |=| geography 'Linestring(90 0 0,0 90 100)'), 6);

SELECT round((tgeogpoint 'Point(-90 0 100)@2000-01-01' |=| geography 'Linestring Z empty'), 6);
SELECT round((tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' |=| geography 'Linestring Z empty'), 6);
SELECT round((tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' |=| geography 'Linestring Z empty'), 6);
SELECT round((tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' |=| geography 'Linestring Z empty'), 6);

SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeompoint 'Point(1 1)@2000-01-01'), 6);
SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'), 6);
SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);

SELECT round((geometry 'Linestring empty' |=| tgeompoint 'Point(1 1)@2000-01-01'), 6);
SELECT round((geometry 'Linestring empty' |=| tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'), 6);
SELECT round((geometry 'Linestring empty' |=| tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round((geometry 'Linestring empty' |=| tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);

SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeompoint 'Point(1 1 1)@2000-01-01'), 6);
SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'), 6);
SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);

SELECT round((geometry 'Linestring Z empty' |=| tgeompoint 'Point(1 1 1)@2000-01-01'), 6);
SELECT round((geometry 'Linestring Z empty' |=| tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'), 6);
SELECT round((geometry 'Linestring Z empty' |=| tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round((geometry 'Linestring Z empty' |=| tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);

SELECT round((geography 'Linestring(90 0,0 90)' |=| tgeogpoint 'Point(-90 0)@2000-01-01'), 6);
SELECT round((geography 'Linestring(90 0,0 90)' |=| tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}'), 6);
SELECT round((geography 'Linestring(90 0,0 90)' |=| tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]'), 6);
SELECT round((geography 'Linestring(90 0,0 90)' |=| tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 6);

SELECT round((geography 'Linestring empty' |=| tgeogpoint 'Point(-90 0)@2000-01-01'), 6);
SELECT round((geography 'Linestring empty' |=| tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}'), 6);
SELECT round((geography 'Linestring empty' |=| tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]'), 6);
SELECT round((geography 'Linestring empty' |=| tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 6);

SELECT round((geography 'Linestring(90 0 0,0 90 100)' |=| tgeogpoint 'Point(-90 0 100)@2000-01-01'), 6);
SELECT round((geography 'Linestring(90 0 0,0 90 100)' |=| tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}'), 6);
SELECT round((geography 'Linestring(90 0 0,0 90 100)' |=| tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]'), 6);
SELECT round((geography 'Linestring(90 0 0,0 90 100)' |=| tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'), 6);

SELECT round((geography 'Linestring Z empty' |=| tgeogpoint 'Point(-90 0 100)@2000-01-01'), 6);
SELECT round((geography 'Linestring Z empty' |=| tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}'), 6);
SELECT round((geography 'Linestring Z empty' |=| tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]'), 6);
SELECT round((geography 'Linestring Z empty' |=| tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'), 6);

SELECT round((tgeompoint 'Point(1 1)@2000-01-01' |=| tgeompoint 'Point(2 2)@2000-01-01'), 6);
SELECT round((tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| tgeompoint 'Point(2 2)@2000-01-01'), 6);
SELECT round((tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| tgeompoint 'Point(2 2)@2000-01-01'), 6);
SELECT round((tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| tgeompoint 'Point(2 2)@2000-01-01'), 6);
SELECT round((tgeompoint 'Point(1 1)@2000-01-01' |=| tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'), 6);
SELECT round((tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'), 6);
SELECT round((tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'), 6);
SELECT round((tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'), 6);
SELECT round((tgeompoint 'Point(1 1)@2000-01-01' |=| tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'), 6);
SELECT round((tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'), 6);
SELECT round((tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'), 6);
SELECT round((tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'), 6);
SELECT round((tgeompoint 'Point(1 1)@2000-01-01' |=| tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round((tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round((tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round((tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);

SELECT round((tgeompoint 'Point(1 1 1)@2000-01-01' |=| tgeompoint 'Point(2 2 2)@2000-01-01'), 6);
SELECT round((tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| tgeompoint 'Point(2 2 2)@2000-01-01'), 6);
SELECT round((tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| tgeompoint 'Point(2 2 2)@2000-01-01'), 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| tgeompoint 'Point(2 2 2)@2000-01-01'), 6);
SELECT round((tgeompoint 'Point(1 1 1)@2000-01-01' |=| tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'), 6);
SELECT round((tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'), 6);
SELECT round((tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'), 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'), 6);
SELECT round((tgeompoint 'Point(1 1 1)@2000-01-01' |=| tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'), 6);
SELECT round((tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'), 6);
SELECT round((tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'), 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'), 6);
SELECT round((tgeompoint 'Point(1 1 1)@2000-01-01' |=| tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round((tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round((tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round((tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);

SELECT round((tgeogpoint 'Point(-90 0)@2000-01-01' |=| tgeogpoint 'Point(0 -90)@2000-01-01'), 6);
SELECT round((tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' |=| tgeogpoint 'Point(0 -90)@2000-01-01'), 6);
SELECT round((tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' |=| tgeogpoint 'Point(0 -90)@2000-01-01'), 6);
SELECT round((tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' |=| tgeogpoint 'Point(0 -90)@2000-01-01'), 6);
SELECT round((tgeogpoint 'Point(-90 0)@2000-01-01' |=| tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'), 6);
SELECT round((tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' |=| tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'), 6);
SELECT round((tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' |=| tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'), 6);
SELECT round((tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' |=| tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'), 6);
SELECT round((tgeogpoint 'Point(-90 0)@2000-01-01' |=| tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'), 6);
SELECT round((tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' |=| tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'), 6);
SELECT round((tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' |=| tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'), 6);
SELECT round((tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' |=| tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'), 6);
SELECT round((tgeogpoint 'Point(-90 0)@2000-01-01' |=| tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 6);
SELECT round((tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' |=| tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 6);
SELECT round((tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' |=| tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 6);
SELECT round((tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' |=| tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 6);

SELECT round((tgeogpoint 'Point(-90 0 100)@2000-01-01' |=| tgeogpoint 'Point(0 -90 100)@2000-01-01'), 6);
SELECT round((tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' |=| tgeogpoint 'Point(0 -90 100)@2000-01-01'), 6);
SELECT round((tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' |=| tgeogpoint 'Point(0 -90 100)@2000-01-01'), 6);
SELECT round((tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' |=| tgeogpoint 'Point(0 -90 100)@2000-01-01'), 6);
SELECT round((tgeogpoint 'Point(-90 0 100)@2000-01-01' |=| tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'), 6);
SELECT round((tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' |=| tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'), 6);
SELECT round((tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' |=| tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'), 6);
SELECT round((tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' |=| tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'), 6);
SELECT round((tgeogpoint 'Point(-90 0 100)@2000-01-01' |=| tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'), 6);
SELECT round((tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' |=| tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'), 6);
SELECT round((tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' |=| tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'), 6);
SELECT round((tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' |=| tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'), 6);
SELECT round((tgeogpoint 'Point(-90 0 100)@2000-01-01' |=| tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'), 6);
SELECT round((tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' |=| tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'), 6);
SELECT round((tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' |=| tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'), 6);
SELECT round((tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' |=| tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'), 6);

/* Errors */
SELECT NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');
SELECT NearestApproachDistance(geometry 'SRID=5676;Linestring(1 1,2 2)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT NearestApproachDistance(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'SRID=5676;Point(1 1)@2000-01-01');
SELECT NearestApproachDistance(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1 1)@2000-01-01');

--------------------------------------------------------

SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));

SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));

SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'));

SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring Z empty'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring Z empty'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty'));

SELECT ST_AsTexT(shortestLine(tgeogpoint 'Point(-90 0)@2000-01-01', geography 'Linestring(90 0,0 90)'), 1);
SELECT ST_AsTexT(shortestLine(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', geography 'Linestring(90 0,0 90)'), 1);
SELECT ST_AsTexT(shortestLine(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', geography 'Linestring(90 0,0 90)'), 1);
SELECT ST_AsTexT(shortestLine(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', geography 'Linestring(90 0,0 90)'), 1);

SELECT ST_AsTexT(shortestLine(tgeogpoint 'Point(-90 0)@2000-01-01', geography 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', geography 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', geography 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', geography 'Linestring empty'));

SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0,3 3)', tgeompoint 'Point(1 1)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0,3 3)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0,3 3)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0,3 3)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT ST_AsTexT(shortestLine(geometry 'Linestring empty', tgeompoint 'Point(1 1)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint 'Point(1 1 1)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));

SELECT ST_AsTexT(shortestLine(geometry 'Linestring Z empty', tgeompoint 'Point(1 1 1)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring Z empty', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring Z empty', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring Z empty', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));

SELECT ST_AsTexT(shortestLine(geography 'Linestring(90 0,0 90)', tgeogpoint 'Point(-90 0)@2000-01-01'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring(90 0,0 90)', tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring(90 0,0 90)', tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring(90 0,0 90)', tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 1);

SELECT ST_AsTexT(shortestLine(geography 'Linestring empty', tgeogpoint 'Point(-90 0)@2000-01-01'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring empty', tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring empty', tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring empty', tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 1);

SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint 'Point(2 2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));

SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0)@2000-01-01', tgeogpoint 'Point(0 -90)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeogpoint 'Point(0 -90)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeogpoint 'Point(0 -90)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeogpoint 'Point(0 -90)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0)@2000-01-01', tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeogpoint '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0)@2000-01-01', tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeogpoint '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0)@2000-01-01', tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeogpoint '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')), 6);

SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0 100)@2000-01-01', tgeogpoint 'Point(0 -90 100)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeogpoint 'Point(0 -90 100)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeogpoint 'Point(0 -90 100)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeogpoint 'Point(0 -90 100)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0 100)@2000-01-01', tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeogpoint '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0 100)@2000-01-01', tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeogpoint '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint 'Point(-90 0 100)@2000-01-01', tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeogpoint '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeogpoint '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')), 6);

SELECT ST_AsTexT(shortestLine(tgeompoint '{[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02], (Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]}', tgeompoint '{[Point(3 3)@2000-01-01, Point(3 3)@2000-01-02], (Point(2 2)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsText(shortestLine(tgeompoint 'Interp=Step;[Point(0 0)@2000-01-01, Point(2 0)@2000-01-02]','[Point(1 1)@2000-01-01,Point(1 1)@2000-01-02]'));
SELECT ST_AsText(shortestLine(tgeompoint 'Interp=Step;{(Point(0 0)@2000-01-01, Point(2 0)@2000-01-02]}','{[Point(1 1)@2000-01-01,Point(1 1)@2000-01-02]}'));
SELECT ST_AsText(shortestLine(tgeompoint '{[Point(0 0)@2000-01-01, Point(2 0)@2000-01-02]}','{(Point(1 1)@2000-01-01,Point(2 2)@2000-01-02]}'));
SELECT ST_AsText(shortestLine(tgeompoint '{[Point(0 0)@2000-01-01],(Point(0 0)@2000-01-02, Point(1 1)@2000-01-03)}','{[Point(1 3)@2000-01-01, Point(1 2)@2000-01-03]}'));
SELECT ST_AsText(shortestLine(tgeompoint '{[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02),[Point(0 0)@2000-01-03]}','{[Point(1 3)@2000-01-01, Point(1 2)@2000-01-03]}'));
SELECT ST_AsText(shortestLine(tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02)', '[Point(4 1)@2000-01-01, Point(2 1)@2000-01-02)'));
-- NULL
SELECT shortestline(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02),[Point(1 1)@2001-01-03, Point(2 2)@2001-01-04]}', tgeompoint '[Point(1 1)@2001-01-02, Point(1 1)@2001-01-03)');
SELECT shortestline(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02),[Point(1 1)@2001-01-03, Point(2 2)@2001-01-04]}', tgeompoint '{[Point(1 1)@2001-01-02, Point(1 1)@2001-01-03)}');
/* Errors */
SELECT shortestLine(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT shortestLine(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');
SELECT shortestLine(geometry 'SRID=5676;Linestring(1 1,2 2)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT shortestLine(geometry 'Linestring(1 1 1,2 2 2)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT shortestLine(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'SRID=5676;Point(1 1)@2000-01-01');
SELECT shortestLine(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT shortestLine(geography 'Linestring(90 0 0,0 90 100)', tgeogpoint 'Point(-90 0 100)@2000-01-01');
SELECT shortestLine(tgeogpoint 'Point(-90 0 100)@2000-01-01', geography 'Linestring(90 0 0,0 90 100)');

--------------------------------------------------------
-- minDistance(tgeompoint[], tgeompoint[]) — exact set-set spatial min distance
--------------------------------------------------------

-- Two singleton sets, segments crossing each other: distance is 0
SELECT minDistance(
  ARRAY[tgeompoint '[Point(0 0)@2000-01-01, Point(2 2)@2000-01-02]'],
  ARRAY[tgeompoint '[Point(0 2)@2000-01-01, Point(2 0)@2000-01-02]']);

-- Two singleton sets, parallel disjoint: exact distance is 1
SELECT round(minDistance(
  ARRAY[tgeompoint '[Point(0 0)@2000-01-01, Point(10 0)@2000-01-02]'],
  ARRAY[tgeompoint '[Point(0 1)@2000-01-01, Point(10 1)@2000-01-02]']), 6);

-- Multi-element sets: closest pair determines the answer, bbox prefilter prunes the rest
SELECT round(minDistance(
  ARRAY[tgeompoint '[Point(0 0)@2000-01-01, Point(1 0)@2000-01-02]',
        tgeompoint '[Point(100 100)@2000-01-01, Point(101 100)@2000-01-02]'],
  ARRAY[tgeompoint '[Point(0 5)@2000-01-01, Point(1 5)@2000-01-02]',
        tgeompoint '[Point(200 200)@2000-01-01, Point(201 200)@2000-01-02]']), 6);

-- Equivalence vs ST_Distance(ST_Collect, ST_Collect): same answer
WITH A AS (SELECT ARRAY[
  tgeompoint '[Point(0 0)@2000-01-01, Point(5 0)@2000-01-02]',
  tgeompoint '[Point(0 10)@2000-01-01, Point(5 10)@2000-01-02]'] AS a),
B AS (SELECT ARRAY[
  tgeompoint '[Point(0 1)@2000-01-01, Point(5 1)@2000-01-02]',
  tgeompoint '[Point(0 11)@2000-01-01, Point(5 11)@2000-01-02]'] AS b)
SELECT round(minDistance(a, b), 6) = round(ST_Distance(
  ST_Collect(ARRAY[trajectory(a[1]), trajectory(a[2])]),
  ST_Collect(ARRAY[trajectory(b[1]), trajectory(b[2])])), 6) AS bit_equivalent
FROM A, B;

-- Errors / NULL handling
SELECT minDistance(ARRAY[]::tgeompoint[], ARRAY[tgeompoint 'Point(0 0)@2000-01-01']);
SELECT minDistance(ARRAY[tgeompoint 'Point(0 0)@2000-01-01'],
                   ARRAY[tgeompoint 'SRID=4326;Point(0 0)@2000-01-01']);

--------------------------------------------------------
-- minDistance(tgeompoint, geometry) — scalar spatial min distance
--------------------------------------------------------

-- Trajectory line segment crossing the geometry point: distance 0
SELECT minDistance(tgeompoint '[Point(0 0)@2000-01-01, Point(2 0)@2000-01-02]',
                   geometry 'Point(1 0)');

-- Diagonal trajectory, side point: perpendicular distance
SELECT round(minDistance(tgeompoint '[Point(0 0)@2000-01-01, Point(10 10)@2000-01-02]',
                         geometry 'Point(5 0)')::numeric, 6);

-- Symmetric (geometry, tgeompoint)
SELECT round(minDistance(geometry 'Point(5 0)',
                         tgeompoint '[Point(0 0)@2000-01-01, Point(10 10)@2000-01-02]')::numeric, 6);

-- Trajectory and a polygon: distance to nearest edge
SELECT round(minDistance(tgeompoint '[Point(0 0)@2000-01-01, Point(10 0)@2000-01-02]',
                         geometry 'POLYGON((20 -5, 30 -5, 30 5, 20 5, 20 -5))')::numeric, 6);

-- SRID mismatch raises
SELECT minDistance(tgeompoint 'Point(0 0)@2000-01-01',
                   geometry 'SRID=4326;Point(1 1)');

--------------------------------------------------------
-- minDistance(tgeompoint, tgeompoint) — 2-ary aggregate
--------------------------------------------------------

-- Aggregate over one row equals the per-pair value
SELECT minDistance(t1, t2) FROM (
  SELECT tgeompoint '[Point(0 0)@2000-01-01, Point(10 10)@2000-01-02]' AS t1,
         tgeompoint '[Point(5 0)@2000-01-01, Point(15 0)@2000-01-02]'  AS t2) v;

-- Aggregate over multiple rows: minimum across all per-pair distances
WITH pairs(t1, t2) AS (VALUES
  (tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]',
   tgeompoint '[Point(10 10)@2000-01-01, Point(11 11)@2000-01-02]'),  -- ~12.7
  (tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]',
   tgeompoint '[Point(0 5)@2000-01-01, Point(1 5)@2000-01-02]'),       -- 4
  (tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]',
   tgeompoint '[Point(2 1)@2000-01-01, Point(2 2)@2000-01-02]'))       -- ~1
SELECT round(minDistance(t1, t2)::numeric, 6) FROM pairs;

-- Grouped: per-group minimum equivalent to MIN over the array form
WITH src(g, t1, t2) AS (VALUES
  (1, tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]',
      tgeompoint '[Point(0 10)@2000-01-01, Point(1 10)@2000-01-02]'),
  (1, tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]',
      tgeompoint '[Point(0 5)@2000-01-01, Point(1 5)@2000-01-02]'),
  (2, tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]',
      tgeompoint '[Point(0 2)@2000-01-01, Point(1 2)@2000-01-02]'))
SELECT g, round(minDistance(t1, t2)::numeric, 6) FROM src GROUP BY g ORDER BY g;

-- Empty group returns the initial-state DBL_MAX (NULL after aggregate finalise
-- semantics is not used here: STYPE=float; PG aggregate of zero rows returns NULL).
SELECT minDistance(t1, t2) FROM (
  SELECT tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]' AS t1,
         tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]' AS t2 WHERE false) v;

--------------------------------------------------------
