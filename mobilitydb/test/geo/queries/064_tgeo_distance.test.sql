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

-- NULL
SELECT round(tgeometry '[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]' <-> tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(2 2)@2000-01-04}', 6);
SELECT round(tgeometry '{[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]}' <-> tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(2 2)@2000-01-04}', 6);
SELECT round(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02],(Point(1 1)@2000-01-03, Point(2 2)@2000-01-04]}' <-> tgeometry '(Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02],(Point(1 1)@2000-01-03, Point(2 2)@2000-01-04]}' <-> tgeometry '{(Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]}', 6);

-- Generic geometries
SELECT round(geometry 'Linestring(1 1,2 2)' <-> tgeometry '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(tgeometry '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> geometry 'Linestring(1 1,2 2)', 6);

/* Errors */
SELECT round(geometry 'srid=5676;Point(2 2)' <-> tgeometry '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(geometry 'Point(2 2 2)' <-> tgeometry '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(tgeometry '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> geometry 'srid=5676;Point(2 2)', 6);
SELECT round(tgeometry '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> geometry 'Point(2 2 2)', 6);
SELECT round(tgeometry 'SRID=5676;[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]' <-> tgeometry '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(tgeometry '[Point(1 1 1)@2000-01-01, Point(3 3 3)@2000-01-03]' <-> tgeometry '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);


SELECT round(geometry 'Point(1 1)' <-> tgeometry 'Point(2 2)@2000-01-01', 6);
SELECT round(geometry 'Point(1 1)' <-> tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(geometry 'Point(1 1)' <-> tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(geometry 'Point(2 2)' <-> tgeometry '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', 6);
SELECT round(geometry 'Point(1 1)' <-> tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);

SELECT round(geometry 'Point empty' <-> tgeometry 'Point(2 2)@2000-01-01', 6);
SELECT round(geometry 'Point empty' <-> tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(geometry 'Point empty' <-> tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(geometry 'Point empty' <-> tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);

SELECT round(geometry 'Point(1 1 1)' <-> tgeometry 'Point(2 2 2)@2000-01-01', 6);
SELECT round(geometry 'Point(1 1 1)' <-> tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(geometry 'Point(1 1 1)' <-> tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(geometry 'Point(1 1 1)' <-> tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);

SELECT round(geometry 'Point Z empty' <-> tgeometry 'Point(2 2 2)@2000-01-01', 6);
SELECT round(geometry 'Point Z empty' <-> tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(geometry 'Point Z empty' <-> tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(geometry 'Point Z empty' <-> tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);

SELECT round(geography 'Point(-90 0)' <-> tgeography 'Point(0 -90)@2000-01-01', 6);
SELECT round(geography 'Point(-90 0)' <-> tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}', 6);
SELECT round(geography 'Point(-90 0)' <-> tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]', 6);
SELECT round(geography 'Point(-90 0)' <-> tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', 6);

SELECT round(geography 'Point empty' <-> tgeography 'Point(0 -90)@2000-01-01', 6);
SELECT round(geography 'Point empty' <-> tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}', 6);
SELECT round(geography 'Point empty' <-> tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]', 6);
SELECT round(geography 'Point empty' <-> tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', 6);

SELECT round(geography 'Point(-90 0 100)' <-> tgeography 'Point(0 -90 100)@2000-01-01', 6);
SELECT round(geography 'Point(-90 0 100)' <-> tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}', 6);
SELECT round(geography 'Point(-90 0 100)' <-> tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]', 6);
SELECT round(geography 'Point(-90 0 100)' <-> tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', 6);

SELECT round(geography 'Point Z empty' <-> tgeography 'Point(0 -90 100)@2000-01-01', 6);
SELECT round(geography 'Point Z empty' <-> tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}', 6);
SELECT round(geography 'Point Z empty' <-> tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]', 6);
SELECT round(geography 'Point Z empty' <-> tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', 6);

SELECT round(tgeometry 'Point(1 1)@2000-01-01' <-> geometry 'Point(1 1)', 6);
SELECT round(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> geometry 'Point(1 1)', 6);
SELECT round(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> geometry 'Point(1 1)', 6);
SELECT round(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> geometry 'Point(1 1)', 6);

SELECT round(tgeometry 'Point(1 1)@2000-01-01' <-> geometry 'Point empty', 6);
SELECT round(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> geometry 'Point empty', 6);
SELECT round(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> geometry 'Point empty', 6);
SELECT round(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> geometry 'Point empty', 6);

SELECT round(tgeometry 'Point(1 1 1)@2000-01-01' <-> geometry 'Point(1 1 1)', 6);
SELECT round(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> geometry 'Point(1 1 1)', 6);
SELECT round(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> geometry 'Point(1 1 1)', 6);
SELECT round(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> geometry 'Point(1 1 1)', 6);

SELECT round(tgeometry 'Point(1 1 1)@2000-01-01' <-> geometry 'Point Z empty', 6);
SELECT round(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> geometry 'Point Z empty', 6);
SELECT round(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> geometry 'Point Z empty', 6);
SELECT round(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> geometry 'Point Z empty', 6);

SELECT round(tgeography 'Point(-90 0)@2000-01-01' <-> geography 'Point(-90 0)', 6);
SELECT round(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' <-> geography 'Point(-90 0)', 6);
SELECT round(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' <-> geography 'Point(-90 0)', 6);
SELECT round(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' <-> geography 'Point(-90 0)', 6);

SELECT round(tgeography 'Point(-90 0)@2000-01-01' <-> geography 'Point empty', 6);
SELECT round(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' <-> geography 'Point empty', 6);
SELECT round(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' <-> geography 'Point empty', 6);
SELECT round(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' <-> geography 'Point empty', 6);

SELECT round(tgeography 'Point(-90 0 100)@2000-01-01' <-> geography 'Point(-90 0 100)', 6);
SELECT round(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' <-> geography 'Point(-90 0 100)', 6);
SELECT round(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' <-> geography 'Point(-90 0 100)', 6);
SELECT round(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' <-> geography 'Point(-90 0 100)', 6);

SELECT round(tgeography 'Point(-90 0 100)@2000-01-01' <-> geography 'Point Z empty', 6);
SELECT round(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' <-> geography 'Point Z empty', 6);
SELECT round(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' <-> geography 'Point Z empty', 6);
SELECT round(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' <-> geography 'Point Z empty', 6);

SELECT round(tgeometry 'Point(1 1)@2000-01-01' <-> tgeometry 'Point(2 2)@2000-01-01', 6);
SELECT round(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeometry 'Point(2 2)@2000-01-01', 6);
SELECT round(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeometry 'Point(2 2)@2000-01-01', 6);
SELECT round(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeometry 'Point(2 2)@2000-01-01', 6);
SELECT round(tgeometry 'Point(1 1)@2000-01-01' <-> tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}', 6);
SELECT round(tgeometry 'Point(1 1)@2000-01-01' <-> tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]', 6);
SELECT round(tgeometry 'Point(1 1)@2000-01-01' <-> tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);
SELECT round(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <-> tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);
SELECT round(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <-> tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);
SELECT round(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <-> tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 6);

SELECT round(tgeometry 'Point(1 1 1)@2000-01-01' <-> tgeometry 'Point(2 2 2)@2000-01-01', 6);
SELECT round(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeometry 'Point(2 2 2)@2000-01-01', 6);
SELECT round(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeometry 'Point(2 2 2)@2000-01-01', 6);
SELECT round(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeometry 'Point(2 2 2)@2000-01-01', 6);
SELECT round(tgeometry 'Point(1 1 1)@2000-01-01' <-> tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}', 6);
SELECT round(tgeometry 'Point(1 1 1)@2000-01-01' <-> tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]', 6);
SELECT round(tgeometry 'Point(1 1 1)@2000-01-01' <-> tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);
SELECT round(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <-> tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);
SELECT round(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <-> tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);
SELECT round(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <-> tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 6);

SELECT round(tgeography 'Point(-90 0)@2000-01-01' <-> tgeography 'Point(0 -90)@2000-01-01', 6);
SELECT round(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' <-> tgeography 'Point(0 -90)@2000-01-01', 6);
SELECT round(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' <-> tgeography 'Point(0 -90)@2000-01-01', 6);
SELECT round(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' <-> tgeography 'Point(0 -90)@2000-01-01', 6);
SELECT round(tgeography 'Point(-90 0)@2000-01-01' <-> tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}', 6);
SELECT round(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' <-> tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}', 6);
SELECT round(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' <-> tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}', 6);
SELECT round(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' <-> tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}', 6);
SELECT round(tgeography 'Point(-90 0)@2000-01-01' <-> tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]', 6);
SELECT round(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' <-> tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]', 6);
SELECT round(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' <-> tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]', 6);
SELECT round(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' <-> tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]', 6);
SELECT round(tgeography 'Point(-90 0)@2000-01-01' <-> tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', 6);
SELECT round(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' <-> tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', 6);
SELECT round(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' <-> tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', 6);
SELECT round(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' <-> tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', 6);

SELECT round(tgeography 'Point(-90 0 100)@2000-01-01' <-> tgeography 'Point(0 -90 100)@2000-01-01', 6);
SELECT round(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' <-> tgeography 'Point(0 -90 100)@2000-01-01', 6);
SELECT round(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' <-> tgeography 'Point(0 -90 100)@2000-01-01', 6);
SELECT round(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' <-> tgeography 'Point(0 -90 100)@2000-01-01', 6);
SELECT round(tgeography 'Point(-90 0 100)@2000-01-01' <-> tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}', 6);
SELECT round(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' <-> tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}', 6);
SELECT round(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' <-> tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}', 6);
SELECT round(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' <-> tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}', 6);
SELECT round(tgeography 'Point(-90 0 100)@2000-01-01' <-> tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]', 6);
SELECT round(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' <-> tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]', 6);
SELECT round(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' <-> tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]', 6);
SELECT round(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' <-> tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]', 6);
SELECT round(tgeography 'Point(-90 0 100)@2000-01-01' <-> tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', 6);
SELECT round(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' <-> tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', 6);
SELECT round(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' <-> tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', 6);
SELECT round(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' <-> tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', 6);

-------------------------------------------------------------------------------

SELECT asText(NearestApproachInstant(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(NearestApproachInstant(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(NearestApproachInstant(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));

SELECT asText(NearestApproachInstant(tgeometry '[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02]', geometry 'Linestring(1 1,3 3)'));

SELECT asText(round(NearestApproachInstant(tgeography 'Point(-90 0)@2000-01-01', geography 'Linestring(90 0,0 90)'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', geography 'Linestring(90 0,0 90)'),6));
SELECT asText(round(NearestApproachInstant(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', geography 'Linestring(90 0,0 90)'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', geography 'Linestring(90 0,0 90)'),6));
SELECT asText(round(NearestApproachInstant(tgeography 'Point(-90 0)@2000-01-01', geography 'Linestring empty'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', geography 'Linestring empty'),6));
SELECT asText(round(NearestApproachInstant(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', geography 'Linestring empty'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', geography 'Linestring empty'),6));

SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeometry 'Point(1 1)@2000-01-01'));
SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asText(NearestApproachInstant(geometry 'Linestring(0 0,3 3)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeometry 'Point(1 1)@2000-01-01'));
SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asText(NearestApproachInstant(geometry 'Linestring empty', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(geography 'Linestring(90 0,0 90)', tgeography 'Point(-90 0)@2000-01-01'));
SELECT asText(NearestApproachInstant(geography 'Linestring(90 0,0 90)', tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}'));
SELECT asText(NearestApproachInstant(geography 'Linestring(90 0,0 90)', tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]'));
SELECT asText(NearestApproachInstant(geography 'Linestring(90 0,0 90)', tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeography 'Point(-90 0)@2000-01-01'));
SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}'));
SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]'));
SELECT asText(NearestApproachInstant(geography 'Linestring empty', tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry 'Point(2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry 'Point(2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry 'Point(2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeometry 'Point(1 1)@2000-01-01', tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeometry 'Point(1 1)@2000-01-01', tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeometry 'Point(1 1)@2000-01-01', tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT asText(NearestApproachInstant(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeometry '[Point(3 3)@2000-01-01, Point(2 2)@2000-01-02]'));
SELECT asText(NearestApproachInstant(tgeometry '{[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02], (Point(1 1)@2000-01-03, Point(0 0)@2000-01-04]}', tgeometry '[Point(3 3)@2000-01-01, Point(3 3)@2000-01-02, Point(2 2)@2000-01-03, Point(3 3)@2000-01-04]'));

SELECT asText(NearestApproachInstant(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry 'Point(2 2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry 'Point(2 2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry 'Point(2 2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry 'Point(2 2 2)@2000-01-01'));
SELECT asText(NearestApproachInstant(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT asText(NearestApproachInstant(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT asText(NearestApproachInstant(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT asText(NearestApproachInstant(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));

SELECT asText(round(NearestApproachInstant(tgeography 'Point(-90 0)@2000-01-01', tgeography 'Point(0 -90)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeography 'Point(0 -90)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeography 'Point(0 -90)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeography 'Point(0 -90)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeography 'Point(-90 0)@2000-01-01', tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeography 'Point(-90 0)@2000-01-01', tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-03]', tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeography 'Point(-90 0)@2000-01-01', tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-03]', tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'),6));

SELECT asText(round(NearestApproachInstant(tgeography 'Point(-90 0 100)@2000-01-01', tgeography 'Point(0 -90 100)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeography 'Point(0 -90 100)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeography 'Point(0 -90 100)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeography 'Point(0 -90 100)@2000-01-01'),6));
SELECT asText(round(NearestApproachInstant(tgeography 'Point(-90 0 100)@2000-01-01', tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}'),6));
SELECT asText(round(NearestApproachInstant(tgeography 'Point(-90 0 100)@2000-01-01', tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-03]', tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]'),6));
SELECT asText(round(NearestApproachInstant(tgeography 'Point(-90 0 100)@2000-01-01', tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-03]', tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'),6));
SELECT asText(round(NearestApproachInstant(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}'),6));

/* Errors */
SELECT NearestApproachInstant(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT NearestApproachInstant(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');
SELECT NearestApproachInstant(geometry 'SRID=5676;Linestring(1 1,2 2)', tgeometry 'Point(1 1)@2000-01-01');
SELECT NearestApproachInstant(geometry 'Linestring(1 1 1,2 2 2)', tgeometry 'Point(1 1)@2000-01-01');
SELECT NearestApproachInstant(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'SRID=5676;Point(1 1)@2000-01-01');
SELECT NearestApproachInstant(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(1 1 1)@2000-01-01');

--------------------------------------------------------

SELECT round(NearestApproachDistance(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)')::numeric, 6);

SELECT round(NearestApproachDistance(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty')::numeric, 6);

SELECT round(NearestApproachDistance(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);

SELECT round(NearestApproachDistance(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Linestring Z empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring Z empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty')::numeric, 6);

SELECT round(NearestApproachDistance(tgeography 'Point(-90 0)@2000-01-01', geography 'Linestring(90 0,0 90)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', geography 'Linestring(90 0,0 90)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', geography 'Linestring(90 0,0 90)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', geography 'Linestring(90 0,0 90)')::numeric, 6);

SELECT round(NearestApproachDistance(tgeography 'Point(-90 0)@2000-01-01', geography 'Linestring empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', geography 'Linestring empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', geography 'Linestring empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', geography 'Linestring empty')::numeric, 6);

SELECT round(NearestApproachDistance(tgeography 'Point(-90 0 100)@2000-01-01', geography 'Linestring(90 0 0,0 90 100)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', geography 'Linestring(90 0 0,0 90 100)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', geography 'Linestring(90 0 0,0 90 100)')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', geography 'Linestring(90 0 0,0 90 100)')::numeric, 6);

SELECT round(NearestApproachDistance(tgeography 'Point(-90 0 100)@2000-01-01', geography 'Linestring Z empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', geography 'Linestring Z empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', geography 'Linestring Z empty')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', geography 'Linestring Z empty')::numeric, 6);

SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeometry 'Point(1 1)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0,3 3)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeometry 'Point(1 1)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring empty', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeometry 'Point(1 1 1)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring(0 0 0,3 3 3)', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeometry 'Point(1 1 1)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geometry 'Linestring Z empty', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(geography 'Linestring(90 0,0 90)', tgeography 'Point(-90 0)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0,0 90)', tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0,0 90)', tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0,0 90)', tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeography 'Point(-90 0)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring empty', tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(geography 'Linestring(90 0 0,0 90 100)', tgeography 'Point(-90 0 100)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0 0,0 90 100)', tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0 0,0 90 100)', tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring(90 0 0,0 90 100)', tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeography 'Point(-90 0 100)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(geography 'Linestring Z empty', tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry 'Point(1 1)@2000-01-01', tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry 'Point(1 1)@2000-01-01', tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry 'Point(1 1)@2000-01-01', tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(tgeography 'Point(-90 0)@2000-01-01', tgeography 'Point(0 -90)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeography 'Point(0 -90)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeography 'Point(0 -90)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeography 'Point(0 -90)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography 'Point(-90 0)@2000-01-01', tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography 'Point(-90 0)@2000-01-01', tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography 'Point(-90 0)@2000-01-01', tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')::numeric, 6);

SELECT round(NearestApproachDistance(tgeography 'Point(-90 0 100)@2000-01-01', tgeography 'Point(0 -90 100)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeography 'Point(0 -90 100)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeography 'Point(0 -90 100)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeography 'Point(0 -90 100)@2000-01-01')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography 'Point(-90 0 100)@2000-01-01', tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography 'Point(-90 0 100)@2000-01-01', tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography 'Point(-90 0 100)@2000-01-01', tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')::numeric, 6);
SELECT round(NearestApproachDistance(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')::numeric, 6);

SELECT round((stbox 'STBOX XT(((0,0),(1,1)),[2000-01-01,2000-01-02])' |=| stbox 'STBOX XT(((2,2),(2,3)),[2000-01-01,2000-01-02])')::numeric, 6);
SELECT round((stbox 'STBOX XT(((0,0),(1,1)),[2000-01-01,2000-01-02])' |=| stbox 'STBOX XT(((2,2),(3,3)),[2000-01-03,2000-01-04])')::numeric, 6);
SELECT round((stbox 'GEODSTBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' |=| stbox 'GEODSTBOX ZT(((2,2,2),(3,3,3)),[2000-01-01,2000-01-02])')::numeric, 6);
-- 3D
SELECT round((stbox 'STBOX ZT(((0,0,0),(1,1,1)),[2000-01-01,2000-01-02])' |=| stbox 'STBOX ZT(((2,2,2),(3,3,3)),[2000-01-01,2000-01-02])')::numeric, 6);

SELECT round((stbox 'STBOX XT(((1,1),(1,1)),[2000-01-01,2000-01-02])' |=| geometry 'Point empty')::numeric, 6);
SELECT round((stbox 'STBOX XT(((1,1),(1,1)),[2000-01-01,2000-01-02])' |=| geometry 'Point(0 0)')::numeric, 6);
SELECT round((stbox 'STBOX ZT(((1,1,1),(1,1,1)),[2000-01-01,2000-01-02])' |=| geometry 'Point (0 0 0)')::numeric, 6);

SELECT round((geometry 'Point empty' |=| stbox 'STBOX XT(((1,1),(1,1)),[2000-01-01,2000-01-02])')::numeric, 6);
SELECT round((geometry 'Point(0 0)' |=| stbox 'STBOX XT(((1,1),(1,1)),[2000-01-01,2000-01-02])')::numeric, 6);
SELECT round((geometry 'Point (0 0 0)' |=| stbox 'STBOX ZT(((1,1,1),(1,1,1)),[2000-01-01,2000-01-02])')::numeric, 6);

/* Errors */
SELECT round((stbox 'STBOX XT(((1,1),(1,1)),[2000-01-01,2000-01-02])' |=| geometry 'SRID=5676;Point(0 0)')::numeric, 6);
SELECT round((stbox 'STBOX XT(((1,1),(1,1)),[2000-01-01,2000-01-02])' |=| geometry 'Point(0 0 0)')::numeric, 6);
SELECT stbox 'STBOX X((1.0,2.0),(3.0,4.0))' |=| stbox 'STBOX Z((1.0,2.0,3.0),(4.0,5.0,6.0))';
SELECT round((stbox 'GEODSTBOX ZT(((1.0,2.0,3.0),(1.0,2.0,3.0)),[2001-01-03,2001-01-04])' |=| geography 'Point(0 0)')::numeric, 6);

SELECT round((tgeometry 'Point(1 1)@2000-01-01' |=| stbox 'STBOX XT(((2,2),(2,2)),[2000-01-01,2000-01-02])')::numeric, 6);
SELECT round((tgeometry 'Point(1 1)@2000-01-01' |=| stbox 'STBOX XT(((2,2),(2,2)),[2000-01-02,2000-01-03])')::numeric, 6);

/* Errors */
SELECT round((tgeometry 'Point(1 1 1)@2000-01-01' |=| stbox 'STBOX XT(((2,2),(2,2)),[2000-01-01,2000-01-02])')::numeric, 6);
SELECT round((tgeography 'Point(1 1)@2000-01-01' |=| stbox 'GEODSTBOX ZT(((2,2,2),(2,2,2)),[2000-01-01,2000-01-02])')::numeric, 6);

SELECT round((stbox 'STBOX XT(((2,2),(2,2)),[2000-01-01,2000-01-02])' |=| tgeometry 'Point(1 1)@2000-01-01' )::numeric, 6);
SELECT round((stbox 'STBOX XT(((2,2),(2,2)),[2000-01-02,2000-01-03])' |=| tgeometry 'Point(1 1)@2000-01-01' )::numeric, 6);

SELECT round((tgeometry 'Point(1 1)@2000-01-01' |=| geometry 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round((tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| geometry 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round((tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| geometry 'Linestring(0 0,3 3)')::numeric, 6);
SELECT round((tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| geometry 'Linestring(0 0,3 3)')::numeric, 6);

SELECT round((tgeometry 'Point(1 1)@2000-01-01' |=| geometry 'Linestring empty')::numeric, 6);
SELECT round((tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| geometry 'Linestring empty')::numeric, 6);
SELECT round((tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| geometry 'Linestring empty')::numeric, 6);
SELECT round((tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| geometry 'Linestring empty')::numeric, 6);

SELECT round((tgeometry 'Point(1 1 1)@2000-01-01' |=| geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round((tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round((tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);
SELECT round((tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| geometry 'Linestring(0 0 0,3 3 3)')::numeric, 6);

SELECT round((tgeometry 'Point(1 1 1)@2000-01-01' |=| geometry 'Linestring Z empty')::numeric, 6);
SELECT round((tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| geometry 'Linestring Z empty')::numeric, 6);
SELECT round((tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| geometry 'Linestring Z empty')::numeric, 6);
SELECT round((tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| geometry 'Linestring Z empty')::numeric, 6);

SELECT round((tgeography 'Point(-90 0)@2000-01-01' |=| geography 'Linestring(90 0,0 90)')::numeric, 6);
SELECT round((tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' |=| geography 'Linestring(90 0,0 90)')::numeric, 6);
SELECT round((tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' |=| geography 'Linestring(90 0,0 90)')::numeric, 6);
SELECT round((tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' |=| geography 'Linestring(90 0,0 90)')::numeric, 6);

SELECT round((tgeography 'Point(-90 0)@2000-01-01' |=| geography 'Linestring empty')::numeric, 6);
SELECT round((tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' |=| geography 'Linestring empty')::numeric, 6);
SELECT round((tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' |=| geography 'Linestring empty')::numeric, 6);
SELECT round((tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' |=| geography 'Linestring empty')::numeric, 6);

SELECT round((tgeography 'Point(-90 0 100)@2000-01-01' |=| geography 'Linestring(90 0 0,0 90 100)')::numeric, 6);
SELECT round((tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' |=| geography 'Linestring(90 0 0,0 90 100)')::numeric, 6);
SELECT round((tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' |=| geography 'Linestring(90 0 0,0 90 100)')::numeric, 6);
SELECT round((tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' |=| geography 'Linestring(90 0 0,0 90 100)')::numeric, 6);

SELECT round((tgeography 'Point(-90 0 100)@2000-01-01' |=| geography 'Linestring Z empty')::numeric, 6);
SELECT round((tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' |=| geography 'Linestring Z empty')::numeric, 6);
SELECT round((tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' |=| geography 'Linestring Z empty')::numeric, 6);
SELECT round((tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' |=| geography 'Linestring Z empty')::numeric, 6);

SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeometry 'Point(1 1)@2000-01-01')::numeric, 6);
SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}')::numeric, 6);
SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')::numeric, 6);
SELECT round((geometry 'Linestring(0 0,3 3)' |=| tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);

SELECT round((geometry 'Linestring empty' |=| tgeometry 'Point(1 1)@2000-01-01')::numeric, 6);
SELECT round((geometry 'Linestring empty' |=| tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}')::numeric, 6);
SELECT round((geometry 'Linestring empty' |=| tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')::numeric, 6);
SELECT round((geometry 'Linestring empty' |=| tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);

SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeometry 'Point(1 1 1)@2000-01-01')::numeric, 6);
SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}')::numeric, 6);
SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')::numeric, 6);
SELECT round((geometry 'Linestring(0 0 0,3 3 3)' |=| tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);

SELECT round((geometry 'Linestring Z empty' |=| tgeometry 'Point(1 1 1)@2000-01-01')::numeric, 6);
SELECT round((geometry 'Linestring Z empty' |=| tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}')::numeric, 6);
SELECT round((geometry 'Linestring Z empty' |=| tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')::numeric, 6);
SELECT round((geometry 'Linestring Z empty' |=| tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);

SELECT round((geography 'Linestring(90 0,0 90)' |=| tgeography 'Point(-90 0)@2000-01-01')::numeric, 6);
SELECT round((geography 'Linestring(90 0,0 90)' |=| tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}')::numeric, 6);
SELECT round((geography 'Linestring(90 0,0 90)' |=| tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]')::numeric, 6);
SELECT round((geography 'Linestring(90 0,0 90)' |=| tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')::numeric, 6);

SELECT round((geography 'Linestring empty' |=| tgeography 'Point(-90 0)@2000-01-01')::numeric, 6);
SELECT round((geography 'Linestring empty' |=| tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}')::numeric, 6);
SELECT round((geography 'Linestring empty' |=| tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]')::numeric, 6);
SELECT round((geography 'Linestring empty' |=| tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')::numeric, 6);

SELECT round((geography 'Linestring(90 0 0,0 90 100)' |=| tgeography 'Point(-90 0 100)@2000-01-01')::numeric, 6);
SELECT round((geography 'Linestring(90 0 0,0 90 100)' |=| tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}')::numeric, 6);
SELECT round((geography 'Linestring(90 0 0,0 90 100)' |=| tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]')::numeric, 6);
SELECT round((geography 'Linestring(90 0 0,0 90 100)' |=| tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')::numeric, 6);

SELECT round((geography 'Linestring Z empty' |=| tgeography 'Point(-90 0 100)@2000-01-01')::numeric, 6);
SELECT round((geography 'Linestring Z empty' |=| tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}')::numeric, 6);
SELECT round((geography 'Linestring Z empty' |=| tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]')::numeric, 6);
SELECT round((geography 'Linestring Z empty' |=| tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')::numeric, 6);

SELECT round((tgeometry 'Point(1 1)@2000-01-01' |=| tgeometry 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| tgeometry 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| tgeometry 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| tgeometry 'Point(2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeometry 'Point(1 1)@2000-01-01' |=| tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeometry 'Point(1 1)@2000-01-01' |=| tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeometry 'Point(1 1)@2000-01-01' |=| tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round((tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' |=| tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round((tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' |=| tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round((tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' |=| tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);

SELECT round((tgeometry 'Point(1 1 1)@2000-01-01' |=| tgeometry 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| tgeometry 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| tgeometry 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| tgeometry 'Point(2 2 2)@2000-01-01')::numeric, 6);
SELECT round((tgeometry 'Point(1 1 1)@2000-01-01' |=| tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')::numeric, 6);
SELECT round((tgeometry 'Point(1 1 1)@2000-01-01' |=| tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')::numeric, 6);
SELECT round((tgeometry 'Point(1 1 1)@2000-01-01' |=| tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round((tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' |=| tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round((tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' |=| tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round((tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' |=| tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);

SELECT round((tgeography 'Point(-90 0)@2000-01-01' |=| tgeography 'Point(0 -90)@2000-01-01')::numeric, 6);
SELECT round((tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' |=| tgeography 'Point(0 -90)@2000-01-01')::numeric, 6);
SELECT round((tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' |=| tgeography 'Point(0 -90)@2000-01-01')::numeric, 6);
SELECT round((tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' |=| tgeography 'Point(0 -90)@2000-01-01')::numeric, 6);
SELECT round((tgeography 'Point(-90 0)@2000-01-01' |=| tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')::numeric, 6);
SELECT round((tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' |=| tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')::numeric, 6);
SELECT round((tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' |=| tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')::numeric, 6);
SELECT round((tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' |=| tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')::numeric, 6);
SELECT round((tgeography 'Point(-90 0)@2000-01-01' |=| tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')::numeric, 6);
SELECT round((tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' |=| tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')::numeric, 6);
SELECT round((tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' |=| tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')::numeric, 6);
SELECT round((tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' |=| tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')::numeric, 6);
SELECT round((tgeography 'Point(-90 0)@2000-01-01' |=| tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')::numeric, 6);
SELECT round((tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}' |=| tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')::numeric, 6);
SELECT round((tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]' |=| tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')::numeric, 6);
SELECT round((tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}' |=| tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')::numeric, 6);

SELECT round((tgeography 'Point(-90 0 100)@2000-01-01' |=| tgeography 'Point(0 -90 100)@2000-01-01')::numeric, 6);
SELECT round((tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' |=| tgeography 'Point(0 -90 100)@2000-01-01')::numeric, 6);
SELECT round((tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' |=| tgeography 'Point(0 -90 100)@2000-01-01')::numeric, 6);
SELECT round((tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' |=| tgeography 'Point(0 -90 100)@2000-01-01')::numeric, 6);
SELECT round((tgeography 'Point(-90 0 100)@2000-01-01' |=| tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')::numeric, 6);
SELECT round((tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' |=| tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')::numeric, 6);
SELECT round((tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' |=| tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')::numeric, 6);
SELECT round((tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' |=| tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')::numeric, 6);
SELECT round((tgeography 'Point(-90 0 100)@2000-01-01' |=| tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')::numeric, 6);
SELECT round((tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' |=| tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')::numeric, 6);
SELECT round((tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' |=| tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')::numeric, 6);
SELECT round((tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' |=| tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')::numeric, 6);
SELECT round((tgeography 'Point(-90 0 100)@2000-01-01' |=| tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')::numeric, 6);
SELECT round((tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}' |=| tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')::numeric, 6);
SELECT round((tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]' |=| tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')::numeric, 6);
SELECT round((tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}' |=| tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')::numeric, 6);

/* Errors */
SELECT NearestApproachDistance(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT NearestApproachDistance(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');
SELECT NearestApproachDistance(geometry 'SRID=5676;Linestring(1 1,2 2)', tgeometry 'Point(1 1)@2000-01-01');
SELECT NearestApproachDistance(geometry 'Linestring(1 1 1,2 2 2)', tgeometry 'Point(1 1)@2000-01-01');
SELECT NearestApproachDistance(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'SRID=5676;Point(1 1)@2000-01-01');
SELECT NearestApproachDistance(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(1 1 1)@2000-01-01');

--------------------------------------------------------

SELECT ST_AsTexT(shortestLine(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(shortestLine(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(shortestLine(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT ST_AsTexT(shortestLine(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));

SELECT ST_AsTexT(shortestLine(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));

SELECT ST_AsTexT(shortestLine(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(shortestLine(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(shortestLine(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT ST_AsTexT(shortestLine(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'));

SELECT ST_AsTexT(shortestLine(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Linestring Z empty'));
SELECT ST_AsTexT(shortestLine(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring Z empty'));
SELECT ST_AsTexT(shortestLine(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty'));
SELECT ST_AsTexT(shortestLine(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty'));

SELECT ST_AsTexT(shortestLine(tgeography 'Point(-90 0)@2000-01-01', geography 'Linestring(90 0,0 90)'), 1);
SELECT ST_AsTexT(shortestLine(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', geography 'Linestring(90 0,0 90)'), 1);
SELECT ST_AsTexT(shortestLine(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', geography 'Linestring(90 0,0 90)'), 1);
SELECT ST_AsTexT(shortestLine(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', geography 'Linestring(90 0,0 90)'), 1);

SELECT ST_AsTexT(shortestLine(tgeography 'Point(-90 0)@2000-01-01', geography 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', geography 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', geography 'Linestring empty'));
SELECT ST_AsTexT(shortestLine(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', geography 'Linestring empty'));

SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0,3 3)', tgeometry 'Point(1 1)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0,3 3)', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0,3 3)', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0,3 3)', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT ST_AsTexT(shortestLine(geometry 'Linestring empty', tgeometry 'Point(1 1)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring empty', tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring empty', tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring empty', tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeometry 'Point(1 1 1)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring(0 0 0,3 3 3)', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));

SELECT ST_AsTexT(shortestLine(geometry 'Linestring Z empty', tgeometry 'Point(1 1 1)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring Z empty', tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring Z empty', tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(geometry 'Linestring Z empty', tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));

SELECT ST_AsTexT(shortestLine(geography 'Linestring(90 0,0 90)', tgeography 'Point(-90 0)@2000-01-01'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring(90 0,0 90)', tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring(90 0,0 90)', tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring(90 0,0 90)', tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 1);

SELECT ST_AsTexT(shortestLine(geography 'Linestring empty', tgeography 'Point(-90 0)@2000-01-01'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring empty', tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring empty', tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]'), 1);
SELECT ST_AsTexT(shortestLine(geography 'Linestring empty', tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}'), 1);

SELECT ST_AsTexT(shortestLine(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry 'Point(2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry 'Point(2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry 'Point(2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeometry 'Point(1 1)@2000-01-01', tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeometry 'Point(1 1)@2000-01-01', tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeometry 'Point(1 1)@2000-01-01', tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT ST_AsTexT(shortestLine(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry 'Point(2 2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry 'Point(2 2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry 'Point(2 2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry 'Point(2 2 2)@2000-01-01'));
SELECT ST_AsTexT(shortestLine(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}'));
SELECT ST_AsTexT(shortestLine(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]'));
SELECT ST_AsTexT(shortestLine(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT ST_AsTexT(shortestLine(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));

SELECT ST_AsTexT(round(shortestLine(tgeography 'Point(-90 0)@2000-01-01', tgeography 'Point(0 -90)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeography 'Point(0 -90)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeography 'Point(0 -90)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeography 'Point(0 -90)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography 'Point(-90 0)@2000-01-01', tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeography '{Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography 'Point(-90 0)@2000-01-01', tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeography '[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography 'Point(-90 0)@2000-01-01', tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03}', tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03]', tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{[Point(-90 0)@2000-01-01, Point(0 0)@2000-01-02, Point(-90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}', tgeography '{[Point(90 0)@2000-01-01, Point(-90 0)@2000-01-02, Point(90 0)@2000-01-03],[Point(90 90)@2000-01-04, Point(90 90)@2000-01-05]}')), 6);

SELECT ST_AsTexT(round(shortestLine(tgeography 'Point(-90 0 100)@2000-01-01', tgeography 'Point(0 -90 100)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeography 'Point(0 -90 100)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeography 'Point(0 -90 100)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeography 'Point(0 -90 100)@2000-01-01')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography 'Point(-90 0 100)@2000-01-01', tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeography '{Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography 'Point(-90 0 100)@2000-01-01', tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeography '[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03]')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography 'Point(-90 0 100)@2000-01-01', tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03}', tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03]', tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')), 6);
SELECT ST_AsTexT(round(shortestLine(tgeography '{[Point(-90 0 100)@2000-01-01, Point(0 0 100)@2000-01-02, Point(-90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}', tgeography '{[Point(90 0 100)@2000-01-01, Point(-90 0 100)@2000-01-02, Point(90 0 100)@2000-01-03],[Point(90 90 100)@2000-01-04, Point(90 90 100)@2000-01-05]}')), 6);

SELECT ST_AsTexT(shortestLine(tgeometry '{[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02], (Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]}', tgeometry '{[Point(3 3)@2000-01-01, Point(3 3)@2000-01-02], (Point(2 2)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsText(shortestLine(tgeometry '{[Point(0 0)@2000-01-01, Point(2 0)@2000-01-02]}','{(Point(1 1)@2000-01-01,Point(2 2)@2000-01-02]}'));
SELECT ST_AsText(shortestLine(tgeometry '{[Point(0 0)@2000-01-01],(Point(1 1)@2000-01-02, Point(1 1)@2000-01-03)}','{[Point(1 3)@2000-01-01, Point(1 2)@2000-01-03]}'));
SELECT ST_AsText(shortestLine(tgeometry '{[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02),[Point(0 0)@2000-01-03]}','{[Point(1 3)@2000-01-01, Point(1 2)@2000-01-03]}'));
SELECT ST_AsText(shortestLine(tgeometry '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02]', '[Point(4 1)@2000-01-01, Point(2 1)@2000-01-02]'));
-- NULL
SELECT shortestline(tgeometry '{[Point(2 2)@2001-01-01, Point(2 2)@2001-01-02),[Point(1 1)@2001-01-03, Point(2 2)@2001-01-04]}', tgeometry '[Point(1 1)@2001-01-02, Point(1 1)@2001-01-03)');
SELECT shortestline(tgeometry '{[Point(1 1)@2001-01-01, Point(1 1)@2001-01-02),[Point(1 1)@2001-01-03, Point(2 2)@2001-01-04]}', tgeometry '{[Point(1 1)@2001-01-02, Point(1 1)@2001-01-03)}');
/* Errors */
SELECT shortestLine(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT shortestLine(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');
SELECT shortestLine(geometry 'SRID=5676;Linestring(1 1,2 2)', tgeometry 'Point(1 1)@2000-01-01');
SELECT shortestLine(geometry 'Linestring(1 1 1,2 2 2)', tgeometry 'Point(1 1)@2000-01-01');
SELECT shortestLine(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'SRID=5676;Point(1 1)@2000-01-01');
SELECT shortestLine(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(1 1 1)@2000-01-01');
SELECT shortestLine(geography 'Linestring(90 0 0,0 90 100)', tgeography 'Point(-90 0 100)@2000-01-01');
SELECT shortestLine(tgeography 'Point(-90 0 100)@2000-01-01', geography 'Linestring(90 0 0,0 90 100)');

--------------------------------------------------------
