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

SELECT geometry 'Point(1 1)'::stbox;
SELECT geometry 'Point(1 1 1)'::stbox;
SELECT round(geography 'Point(1 1)'::stbox, 13);
SELECT round(geography 'Point(1 1 1)'::stbox, 13);
SELECT timestamptz '2000-01-01'::stbox;
SELECT tstzset '{2000-01-01, 2000-01-02}'::stbox;
SELECT tstzspan '[2000-01-01, 2000-01-02]'::stbox;
SELECT tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-03, 2000-01-04]}'::stbox;
SELECT stbox(geometry 'Point(1 1)', timestamptz '2000-01-01');
SELECT stbox(geometry 'Point(1 1 1)', timestamptz '2000-01-01');
SELECT stbox(geometry 'Point(1 1)', tstzspan '[2000-01-01, 2000-01-02]');
SELECT stbox(geometry 'Point(1 1 1)', tstzspan '[2000-01-01, 2000-01-02]');
SELECT round(stbox(geography 'Point(1 1)', timestamptz '2000-01-01'), 13);
SELECT round(stbox(geography 'Point(1 1 1)', timestamptz '2000-01-01'), 13);
SELECT round(stbox(geography 'Point(1 1)', tstzspan '[2000-01-01, 2000-01-02]'), 13);
SELECT round(stbox(geography 'Point(1 1 1)', tstzspan '[2000-01-01, 2000-01-02]'), 13);
-- Empty geometry
SELECT geometry 'Point empty'::stbox;
SELECT geography 'Point empty'::stbox;
SELECT stbox(geometry 'Point empty', timestamptz '2000-01-01');
SELECT stbox(geometry 'Point empty', tstzspan '[2000-01-01, 2000-01-02]');
SELECT stbox(geography 'Point empty', timestamptz '2000-01-01');
SELECT stbox(geography 'Point empty', tstzspan '[2000-01-01, 2000-01-02]');

SELECT tgeompoint 'Point(1 1)@2000-01-01'::stbox;
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'::stbox;
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'::stbox;
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'::stbox;

SELECT round(tgeogpoint 'Point(1 1)@2000-01-01'::stbox, 13);
SELECT round(tgeogpoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'::stbox, 13);
SELECT round(tgeogpoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'::stbox, 13);
SELECT round(tgeogpoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'::stbox, 13);

-------------------------------------------------------------------------------

SELECT stboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)');
SELECT stboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2,5 1,6 2,7 1,8 2,9 1,10 2))');
-- 3D
SELECT stboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))');

/* Errors */
SELECT stboxes(geometry 'Polygon((1 1,1 2,2 2,2 1,1 1))');
SELECT stboxes(geometry 'Linestring empty');

-------------------------------------------------------------------------------

SELECT stboxes(tgeompoint '[Point(1 1)@2000-01-01]');
SELECT stboxes(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT stboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT stboxes(tgeompoint 'Point(1 1)@2000-01-01');
SELECT stboxes(tgeompoint '{Point(1 1)@2000-01-01}');

-------------------------------------------------------------------------------

-- Linestring
SELECT splitNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 1);
SELECT splitNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 2);
SELECT splitNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 3);
SELECT splitNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 4);
SELECT splitNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 5);
SELECT splitNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 6);
--3D
SELECT splitNStboxes(geometry 'Linestring(1 1 1,2 2 1,3 1 1,4 2 1,5 1 1)', 1);
SELECT splitNStboxes(geometry 'Linestring(1 1 1,2 2 1,3 1 1,4 2 1,5 1 1)', 2);
SELECT splitNStboxes(geometry 'Linestring(1 1 1,2 2 1,3 1 1,4 2 1,5 1 1)', 3);
SELECT splitNStboxes(geometry 'Linestring(1 1 1,2 2 1,3 1 1,4 2 1,5 1 1)', 4);
SELECT splitNStboxes(geometry 'Linestring(1 1 1,2 2 1,3 1 1,4 2 1,5 1 1)', 5);
SELECT splitNStboxes(geometry 'Linestring(1 1 1,2 2 1,3 1 1,4 2 1,5 1 1)', 6);
--4D
SELECT splitNStboxes(geometry 'Linestring(1 1 1 1,2 2 1 1,3 1 1 1,4 2 1 1,5 1 1 1)', 1);
SELECT splitNStboxes(geometry 'Linestring(1 1 1 1,2 2 1 1,3 1 1 1,4 2 1 1,5 1 1 1)', 2);
SELECT splitNStboxes(geometry 'Linestring(1 1 1 1,2 2 1 1,3 1 1 1,4 2 1 1,5 1 1 1)', 3);
SELECT splitNStboxes(geometry 'Linestring(1 1 1 1,2 2 1 1,3 1 1 1,4 2 1 1,5 1 1 1)', 4);
SELECT splitNStboxes(geometry 'Linestring(1 1 1 1,2 2 1 1,3 1 1 1,4 2 1 1,5 1 1 1)', 5);
SELECT splitNStboxes(geometry 'Linestring(1 1 1 1,2 2 1 1,3 1 1 1,4 2 1 1,5 1 1 1)', 6);

-- Multilinestring
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2,5 1,6 2,7 1,8 2,9 1,10 2))', 1);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2,5 1,6 2,7 1,8 2,9 1,10 2))', 2);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2,5 1,6 2,7 1,8 2,9 1,10 2))', 3);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2,5 1,6 2,7 1,8 2,9 1,10 2))', 4);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2,5 1,6 2,7 1,8 2,9 1,10 2))', 5);

SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2,7 1,8 2,9 1,10 2))', 1);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2,7 1,8 2,9 1,10 2))', 2);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2,7 1,8 2,9 1,10 2))', 3);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2,7 1,8 2,9 1,10 2))', 4);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2,7 1,8 2,9 1,10 2))', 5);

SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1),(4 2,5 1,6 2),(7 1,8 2,9 1,10 2))', 1);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1),(4 2,5 1,6 2),(7 1,8 2,9 1,10 2))', 2);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1),(4 2,5 1,6 2),(7 1,8 2,9 1,10 2))', 3);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1),(4 2,5 1,6 2),(7 1,8 2,9 1,10 2))', 4);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1),(4 2,5 1,6 2),(7 1,8 2,9 1,10 2))', 5);

SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 1);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 2);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 3);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 4);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 5);

SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2),(3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 1);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2),(3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 2);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2),(3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 3);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2),(3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 4);
SELECT splitNStboxes(geometry 'Multilinestring((1 1,2 2),(3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 5);

-- 3D
SELECT splitNStboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))', 1);
SELECT splitNStboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))', 2);
SELECT splitNStboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))', 3);
SELECT splitNStboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))', 4);
SELECT splitNStboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))', 5);
SELECT splitNStboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))', 6);
SELECT splitNStboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))', 12);

/* Errors */
SELECT splitNStboxes(geometry 'Polygon((1 1,1 2,2 2,2 1,1 1))', 1);
SELECT splitNStboxes(geometry 'Linestring(1 1,1 2,2 2,2 1,1 1)', -1);
SELECT splitNStboxes(geometry 'Linestring empty', 1);

-------------------------------------------------------------------------------

-- Sequence
SELECT splitNStboxes(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05]', 1);
SELECT splitNStboxes(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05]', 2);
SELECT splitNStboxes(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05]', 3);
SELECT splitNStboxes(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05]', 4);
SELECT splitNStboxes(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05]', 5);
SELECT splitNStboxes(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05]', 6);
-- Sequence set
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06, Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09,Point(10 2)@2000-01-10]}', 1);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06, Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09,Point(10 2)@2000-01-10]}', 2);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06, Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09,Point(10 2)@2000-01-10]}', 3);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06, Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09,Point(10 2)@2000-01-10]}', 4);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06, Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09,Point(10 2)@2000-01-10]}', 5);

SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01], [Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05], [Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07],
[Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 1);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01], [Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05], [Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07],
[Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 2);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01], [Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05], [Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07],
[Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 3);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01], [Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05], [Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07],
[Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 4);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01], [Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05], [Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07],
[Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 5);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01], [Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05], [Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07],
[Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 6);

SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 1);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 2);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 3);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 4);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 5);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 6);

SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 1);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 2);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 3);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 4);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 5);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 6);

SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 1);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 2);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 3);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 4);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 5);
SELECT splitNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 6);
/* Errors */
SELECT splitNStboxes(tgeompoint 'Point(1 1)@2000-01-01', -1);

-------------------------------------------------------------------------------

-- Linestring
SELECT splitEachNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 1);
SELECT splitEachNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 2);
SELECT splitEachNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 3);
SELECT splitEachNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 4);
SELECT splitEachNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 5);
SELECT splitEachNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 6);
--3D
SELECT splitEachNStboxes(geometry 'Linestring(1 1 1,2 2 1,3 1 1,4 2 1,5 1 1)', 1);
SELECT splitEachNStboxes(geometry 'Linestring(1 1 1,2 2 1,3 1 1,4 2 1,5 1 1)', 2);
SELECT splitEachNStboxes(geometry 'Linestring(1 1 1,2 2 1,3 1 1,4 2 1,5 1 1)', 3);
SELECT splitEachNStboxes(geometry 'Linestring(1 1 1,2 2 1,3 1 1,4 2 1,5 1 1)', 4);
SELECT splitEachNStboxes(geometry 'Linestring(1 1 1,2 2 1,3 1 1,4 2 1,5 1 1)', 5);
SELECT splitEachNStboxes(geometry 'Linestring(1 1 1,2 2 1,3 1 1,4 2 1,5 1 1)', 6);
--4D
SELECT splitEachNStboxes(geometry 'Linestring(1 1 1 1,2 2 1 1,3 1 1 1,4 2 1 1,5 1 1 1)', 1);
SELECT splitEachNStboxes(geometry 'Linestring(1 1 1 1,2 2 1 1,3 1 1 1,4 2 1 1,5 1 1 1)', 2);
SELECT splitEachNStboxes(geometry 'Linestring(1 1 1 1,2 2 1 1,3 1 1 1,4 2 1 1,5 1 1 1)', 3);
SELECT splitEachNStboxes(geometry 'Linestring(1 1 1 1,2 2 1 1,3 1 1 1,4 2 1 1,5 1 1 1)', 4);
SELECT splitEachNStboxes(geometry 'Linestring(1 1 1 1,2 2 1 1,3 1 1 1,4 2 1 1,5 1 1 1)', 5);
SELECT splitEachNStboxes(geometry 'Linestring(1 1 1 1,2 2 1 1,3 1 1 1,4 2 1 1,5 1 1 1)', 6);

-- Multilinestring
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2,5 1,6 2,7 1,8 2,9 1,10 2))', 1);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2,5 1,6 2,7 1,8 2,9 1,10 2))', 2);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2,5 1,6 2,7 1,8 2,9 1,10 2))', 3);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2,5 1,6 2,7 1,8 2,9 1,10 2))', 4);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2,5 1,6 2,7 1,8 2,9 1,10 2))', 5);

SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2,7 1,8 2,9 1,10 2))', 1);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2,7 1,8 2,9 1,10 2))', 2);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2,7 1,8 2,9 1,10 2))', 3);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2,7 1,8 2,9 1,10 2))', 4);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2,7 1,8 2,9 1,10 2))', 5);

SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1),(4 2,5 1,6 2),(7 1,8 2,9 1,10 2))', 1);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1),(4 2,5 1,6 2),(7 1,8 2,9 1,10 2))', 2);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1),(4 2,5 1,6 2),(7 1,8 2,9 1,10 2))', 3);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1),(4 2,5 1,6 2),(7 1,8 2,9 1,10 2))', 4);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1),(4 2,5 1,6 2),(7 1,8 2,9 1,10 2))', 5);

SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 1);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 2);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 3);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 4);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2,3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 5);

SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2),(3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 1);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2),(3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 2);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2),(3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 3);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2),(3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 4);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1,2 2),(3 1,4 2),(5 1,6 2),(7 1,8 2),(9 1,10 2))', 5);

-- 3D
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))', 1);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))', 2);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))', 3);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))', 4);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))', 5);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))', 6);
SELECT splitEachNStboxes(geometry 'Multilinestring((1 1 1,2 2 1,3 1 1,4 2 1,5 1 1,6 2 1,7 1 1,8 2 1,9 1 1,10 2 1))', 12);

/* Errors */
SELECT splitEachNStboxes(geometry 'Polygon((1 1,1 2,2 2,2 1,1 1))', 1);
SELECT splitEachNStboxes(geometry 'Linestring(1 1,1 2,2 2,2 1,1 1)', -1);
SELECT splitEachNStboxes(geometry 'Linestring empty', 1);

-------------------------------------------------------------------------------

-- Instant and instantaneous sequence and sequence set
SELECT splitEachNStboxes(tgeompoint 'Point(1 1)@2000-01-01', 1);
SELECT splitEachNStboxes(tgeompoint '{Point(1 1)@2000-01-01}', 1);
SELECT splitEachNStboxes(tgeompoint '[Point(1 1)@2000-01-01]', 1);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01]}', 1);
-- Discrete Sequence
SELECT splitEachNStboxes(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05}', 1);
SELECT splitEachNStboxes(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05}', 2);
SELECT splitEachNStboxes(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05}', 3);
SELECT splitEachNStboxes(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05}', 4);
SELECT splitEachNStboxes(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05}', 5);
SELECT splitEachNStboxes(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05}', 6);
-- Sequence
SELECT splitEachNStboxes(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05]', 1);
SELECT splitEachNStboxes(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05]', 2);
SELECT splitEachNStboxes(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05]', 3);
SELECT splitEachNStboxes(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05]', 4);
SELECT splitEachNStboxes(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05]', 5);
SELECT splitEachNStboxes(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03, Point(4 2)@2000-01-04, Point(5 1)@2000-01-05]', 6);
-- Sequence set
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06, Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09,Point(10 2)@2000-01-10]}', 1);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06, Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09,Point(10 2)@2000-01-10]}', 2);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06, Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09,Point(10 2)@2000-01-10]}', 3);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06, Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09,Point(10 2)@2000-01-10]}', 4);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06, Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09,Point(10 2)@2000-01-10]}', 5);

SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01], [Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05], [Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07],
[Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 1);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01], [Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05], [Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07],
[Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 2);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01], [Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05], [Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07],
[Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 3);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01], [Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05], [Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07],
[Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 4);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01], [Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05], [Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07],
[Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 5);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01], [Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05], [Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07],
[Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 6);

SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 1);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 2);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 3);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 4);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 5);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04], [Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08], [Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 6);

SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 1);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 2);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 3);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 4);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 5);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03],
[Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09], [Point(10 2)@2000-01-10]}', 6);

SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 1);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 2);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 3);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 4);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 5);
SELECT splitEachNStboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03,
Point(4 2)@2000-01-04, Point(5 1)@2000-01-05, Point(6 2)@2000-01-06], [Point(7 1)@2000-01-07,
Point(8 2)@2000-01-08, Point(9 1)@2000-01-09, Point(10 2)@2000-01-10]}', 6);

/* Errors */
SELECT splitEachNStboxes(tgeompoint 'Point(1 1)@2000-01-01', -1);

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeompoint WHERE temp::stbox IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE temp::stbox IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint WHERE splitNStboxes(temp, 1) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint WHERE splitNStboxes(temp, 2) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint WHERE splitNStboxes(temp, 3) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint WHERE splitNStboxes(temp, 4) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint WHERE splitNStboxes(temp, 5) IS NOT NULL;

-------------------------------------------------------------------------------

SELECT expandSpace(stbox 'STBOX X((1.0,2.0),(1.0,2.0))', 0.5);
SELECT expandSpace(stbox 'STBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))', 0.5);
SELECT expandSpace(stbox 'STBOX XT(((1.0,2.0),(1.0,2.0)),[2000-01-03,2000-01-03])', 0.5);
SELECT expandSpace(stbox 'STBOX ZT(((1.0,2.0,3.0),(1.0,2.0,3.0)),[2000-01-03,2000-01-04])', 0.5);
SELECT expandSpace(stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))', 0.5);

SELECT expandTime(stbox 'STBOX XT(((1.0,2.0),(1.0,2.0)),[2000-01-03,2000-01-03])', '1 day');
SELECT expandTime(stbox 'STBOX ZT(((1.0,2.0,3.0),(1.0,2.0,3.0)),[2000-01-04,2000-01-04])', '1 day');
SELECT expandTime(stbox 'GEODSTBOX ZT(((1.0,2.0,3.0),(1.0,2.0,3.0)),[2000-01-04,2000-01-04])', '1 day');
SELECT expandTime(stbox 'STBOX XT(((1.0,2.0),(1.0,2.0)),[2000-01-01,2000-01-03])', '-1 day');
SELECT expandTime(stbox 'STBOX ZT(((1.0,2.0,3.0),(1.0,2.0,3.0)),[2000-01-01,2000-01-03])', '-1 day');
SELECT expandTime(stbox 'GEODSTBOX ZT(((1.0,2.0,3.0),(1.0,2.0,3.0)),[2000-01-01,2000-01-03])', '-1 day');
-- NULL
SELECT expandTime(stbox 'STBOX XT(((1.0,2.0),(1.0,2.0)),[2000-01-02,2000-01-03])', '-1 day');
/* Errors */
SELECT expandTime(stbox 'STBOX X((1.0,2.0),(1.0,2.0))', '1 day');
SELECT expandTime(stbox 'STBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))', '1 day');
SELECT expandTime(stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))', '1 day');

SELECT expandSpace(geometry 'Linestring(1 1,2 2)', 0.5);
-- NULL
SELECT expandSpace(geometry 'Linestring empty', 0.5);
SELECT expandSpace(geography 'Linestring empty', 0.5);

-------------------------------------------------------------------------------

SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' && tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' && tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' && tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' && tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
/* Errors */
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' && tgeompoint 'Point(1 1 1)@2000-01-01';

SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' && tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' && tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' && tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' && tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' && tstzspan '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && tstzspan '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1)@2000-01-01' && stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' && stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' && stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' && stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' && stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' && stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';

SELECT tgeompoint 'Point(1 1)@2000-01-01' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

-- Mixed 2D/3D
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && tgeompoint 'Point(1 1)@2000-01-01';

/* Errors */
SELECT tgeompoint 'SRID=5676;Point(1 1)@2000-01-01' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1,1),(2,2)),[2001-01-01,2001-01-02])' && stbox 'STBOX XT(((1,1),(2,2)),[2001-01-01,2001-01-02])';
SELECT stbox 'GEODSTBOX ZT(((1,1,1),(2,2,2)),[2001-01-01,2001-01-02])' && stbox 'STBOX XT(((1,1),(2,2)),[2001-01-01,2001-01-02])';
SELECT tgeompoint 'SRID=5676;Point(1 1)@2000-01-01' && stbox 'STBOX XT(((1,1),(2,2)),[2001-01-01,2001-01-02])';

-------------------------------------------------------------------------------

SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' @> tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' @> tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' @> tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' @> tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' @> tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' @> tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' @> tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' @> tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' @> tstzspan '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> tstzspan '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1)@2000-01-01' @> stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' @> stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' @> stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' @> stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' @> stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' @> stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';

SELECT tgeompoint 'Point(1 1)@2000-01-01' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

-- Mixed 2D/3D
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> tgeompoint 'Point(1 1)@2000-01-01';

/* Errors */
SELECT tgeompoint 'SRID=5676;Point(1 1)@2000-01-01' @> tgeompoint 'Point(1 1)@2000-01-01';

-------------------------------------------------------------------------------

SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' <@ tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' <@ tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' <@ tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' <@ tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' <@ tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' <@ tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' <@ tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' <@ tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <@ tstzspan '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ tstzspan '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <@ stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <@ stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <@ stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <@ stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' <@ stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

-- Mixed 2D/3D
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ tgeompoint 'Point(1 1)@2000-01-01';

/* Errors */
SELECT tgeompoint 'SRID=5676;Point(1 1)@2000-01-01' <@ tgeompoint 'Point(1 1)@2000-01-01';

-------------------------------------------------------------------------------

SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' -|- tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' -|- tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' -|- tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' -|- tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' -|- tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' -|- tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' -|- tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' -|- tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' -|- tstzspan '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- tstzspan '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' -|- stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' -|- stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' -|- stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' -|- stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' -|- stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';

SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

-- Mixed 2D/3D
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- tgeompoint 'Point(1 1)@2000-01-01';

/* Errors */
SELECT tgeompoint 'SRID=5676;Point(1 1)@2000-01-01' -|- tgeompoint 'Point(1 1)@2000-01-01';

-------------------------------------------------------------------------------

SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' ~= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' ~= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' ~= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' ~= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' ~= tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' ~= tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' ~= tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))' ~= tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' ~= tstzspan '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= tstzspan '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= stbox 'STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' ~= stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' ~= stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' ~= stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' ~= stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= stbox 'STBOX Z((1.0,2.0,2.0),(1.0,2.0,2.0))';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' ~= stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= stbox 'GEODSTBOX Z((1.0,2.0,3.0),(1.0,2.0,3.0))';

SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

-- Mixed 2D/3D
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= tgeompoint 'Point(1 1)@2000-01-01';

/* Errors */
SELECT tgeompoint 'SRID=5676;Point(1 1)@2000-01-01' ~= tgeompoint 'Point(1 1)@2000-01-01';

-------------------------------------------------------------------------------
