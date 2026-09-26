-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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
-- Multidimensional tiling
-------------------------------------------------------------------------------

SELECT spaceTiles(tgeompoint '[Point(3 3)@2001-01-15, Point(15 15)@2001-01-25]'::stbox, 2.0) LIMIT 3;
SELECT spaceTiles(tgeompoint 'SRID=3812;[Point(3 3)@2001-01-15, Point(15 15)@2001-01-25]'::stbox, 2.0, geometry 'Point(3 3)') LIMIT 3;
SELECT spaceTiles(tgeompoint '[Point(3 3 3)@2001-01-15, Point(15 15 15)@2001-01-25]'::stbox, 2.0, geometry 'Point(3 3 3)') LIMIT 3;
SELECT spaceTimeTiles(tgeompoint '[Point(3 3)@2001-01-15, Point(15 15)@2001-01-25]'::stbox, 2.0, interval '2 days', 'Point(3 3)', '2001-01-15') LIMIT 3;
SELECT spaceTimeTiles(tgeompoint '[Point(3 3 3)@2001-01-15, Point(15 15 15)@2001-01-25]'::stbox, 2.0, interval '2 days', 'Point(3 3 3)', '2001-01-15') LIMIT 3;
/* Errors */
SELECT spaceTiles(tgeompoint '[Point(3 3 3)@2001-01-15, Point(15 15 15)@2001-01-25]'::stbox, 2.0, geometry 'Point(3 3)');
SELECT spaceTiles(tgeompoint 'SRID=3812;[Point(3 3)@2001-01-15, Point(15 15)@2001-01-25]'::stbox, 2.0, geometry 'SRID=5676;Point(1 1)');
SELECT spaceTiles(tgeogpoint '[Point(3 3)@2001-01-15, Point(15 15)@2001-01-25]'::stbox, 2.0);

SELECT getSpaceTile(geometry 'Point(3 3)', 2.0);
SELECT getSpaceTile(geometry 'Point(3 3 3)', 2.0);
SELECT getStboxTimeTile(timestamptz '2001-01-15', interval '2 days');
SELECT getStboxTimeTile(timestamptz '2001-01-15', interval '2 days', '2020-06-15');
SELECT getSpaceTimeTile(geometry 'Point(3 3)', timestamptz '2001-01-15', 2.0, interval '2 days');
SELECT getSpaceTimeTile(geometry 'Point(3 3)', timestamptz '2001-01-15', 2.0, interval '2 days');
SELECT getSpaceTimeTile(geometry 'Point(3 3 3)', timestamptz '2001-01-15', 2.0, interval '2 days', geometry 'Point(1 1 1)', '2020-06-15');

SELECT getSpaceTimeTile(geometry 'SRID=3812;Point(3 3 3)', timestamptz '2001-01-15', 2.0, interval '2 days', geometry 'SRID=3812;Point(1 1 1)', '2020-06-15');
/* Errors */
SELECT getSpaceTimeTile(geometry 'Point(3 3 3)', timestamptz '2001-01-15', 2.0, interval '2 days', geometry 'Point(1 1)', '2020-06-15');
SELECT getSpaceTimeTile(geometry 'SRID=3812;Point(3 3 3)', timestamptz '2001-01-15', 2.0, interval '2 days', geometry 'SRID=2154;Point(1 1)', '2020-06-15');

-------------------------------------------------------------------------------
-- Space boxes
-------------------------------------------------------------------------------

SELECT round(spaceBoxes(tgeompoint '[Point(1 1)@2001-01-01, Point(10 10)@2001-01-10]', 2.0), 6);
SELECT round(spaceBoxes(tgeompoint 'SRID=3812;[Point(1 1)@2001-01-01, Point(10 10)@2001-01-10]', 2.0, geometry 'Point(1 1)'), 6);
SELECT round(spaceBoxes(tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-10]', 2.0, geometry 'Point(1 1 1)'), 6);

/* Errors */
SELECT spaceBoxes(tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-10]', 2.0, geometry 'Point(1 1)');
SELECT spaceBoxes(tgeompoint 'SRID=3812;[Point(1 1)@2001-01-01, Point(10 10)@2001-01-10]', 2.0, geometry 'SRID=5676;Point(1 1)');

-------------------------------------------------------------------------------
-- time boxes
-------------------------------------------------------------------------------

SELECT round(timeBoxes(tgeompoint '[Point(1 1)@2001-01-01, Point(10 10)@2001-01-10]', interval '2 days', '2001-01-01'), 6);
SELECT round(timeBoxes(tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-10]', interval '2 days', '2001-01-01'));

/* Errors */
SELECT timeBoxes(tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-10]', interval '2 months', '2001-01-01');

-------------------------------------------------------------------------------
-- SpaceTime boxes
-------------------------------------------------------------------------------

SELECT round(spaceTimeBoxes(tgeompoint '[Point(1 1)@2001-01-01, Point(10 10)@2001-01-10]', 2.0, interval '2 days', 'Point(1 1)', '2001-01-01'), 6);
SELECT round(spaceTimeBoxes(tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-10]', 2.0, interval '2 days', 'Point(1 1 1)', '2001-01-01'));

/* Errors */
SELECT spaceTimeBoxes(tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-10]', 2.0, interval '2 days', 'Point(1 1)', '2001-01-01');
SELECT spaceTimeBoxes(tgeompoint 'SRID=3812;[Point(3 3)@2001-01-15, Point(15 15)@2001-01-25]', 2.0, interval '2 days', 'SRID=5676;Point(1 1)', '2001-01-01');

-------------------------------------------------------------------------------
-- Space split
-------------------------------------------------------------------------------

-- 2D
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint 'Point(1 1)@2001-01-01', 2.0) AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', 2.0) AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', 2.0) AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', 2.0) AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', 2.0) AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint 'Interp=Step;{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', 2.0) AS sp) t;

SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint 'Point(1 1)@2001-01-01', 2.0, geometry 'Point(0.5 0.5)') AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', 2.0, geometry 'Point(0.5 0.5)') AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', 2.0, geometry 'Point(0.5 0.5)') AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', 2.0, geometry 'Point(0.5 0.5)') AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', 2.0, geometry 'Point(0.5 0.5)') AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint 'Interp=Step;{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', 2.0, geometry 'Point(0.5 0.5)') AS sp) t;

-- 3D
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint 'Point(1 1 1)@2001-01-01', 2.0) AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', 2.0) AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', 2.0) AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', 2.0) AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint 'Interp=Step;[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', 2.0) AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint 'Interp=Step;{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', 2.0) AS sp) t;

SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint 'Point(1 1 1)@2001-01-01', 2.0, geometry 'Point(0.5 0.5 0.5)') AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', 2.0, geometry 'Point(0.5 0.5 0.5)') AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', 2.0, geometry 'Point(0.5 0.5 0.5)') AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', 2.0, geometry 'Point(0.5 0.5 0.5)') AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint 'Interp=Step;[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', 2.0, geometry 'Point(0.5 0.5 0.5)') AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint 'Interp=Step;{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', 2.0, geometry 'Point(0.5 0.5 0.5)') AS sp) t;

SELECT ST_AsText((sp).point) AS point, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceSplit(tgeompoint '[Point(1 1 1)@2001-01-01, Point(3 1 1)@2001-01-03, Point(3 1 3)@2001-01-05]',
2.0, bitmatrix := false) AS sp) t;

/* Errors */
SELECT spaceSplit(tgeompoint 'SRID=5676;Point(1 1 1)@2001-01-01', 2.0, geometry 'SRID=3812;Point(0.5 0.5 0.5)');

-------------------------------------------------------------------------------
-- Space-time split
-------------------------------------------------------------------------------

-- Without bitmatrix
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint 'Point(1 1)@2001-01-01', 2.0, interval '2 days', bitmatrix:=false) AS sp) t;

-- 2D
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint 'Point(1 1)@2001-01-01', 2.0, interval '2 days') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', 2.0, interval '2 days') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', 2.0, interval '2 days') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', 2.0, interval '2 days') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', 2.0, interval '2 days') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint 'Interp=Step;{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', 2.0, interval '2 days') AS sp) t;

SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint 'Point(1 1)@2001-01-01', 2.0, interval '2 days', 'Point(0.5 0.5)', '2001-01-15') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}', 2.0, interval '2 days', 'Point(0.5 0.5)', '2001-01-15') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', 2.0, interval '2 days', 'Point(0.5 0.5)', '2001-01-15') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', 2.0, interval '2 days', 'Point(0.5 0.5)', '2001-01-15') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]', 2.0, interval '2 days', 'Point(0.5 0.5)', '2001-01-15') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint 'Interp=Step;{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}', 2.0, interval '2 days', 'Point(0.5 0.5)', '2001-01-15') AS sp) t;

-- 3D
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint 'Point(1 1 1)@2001-01-01', 2.0, interval '2 days') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', 2.0, interval '2 days') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', 2.0, interval '2 days') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', 2.0, interval '2 days') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint 'Interp=Step;[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', 2.0, interval '2 days') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint 'Interp=Step;{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', 2.0, interval '2 days') AS sp) t;

SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint 'Point(1 1 1)@2001-01-01', 2.0, interval '2 days', 'Point(0.5 0.5 0.5)', '2001-01-15') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}', 2.0, interval '2 days', 'Point(0.5 0.5 0.5)', '2001-01-15') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', 2.0, interval '2 days', 'Point(0.5 0.5 0.5)', '2001-01-15') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', 2.0, interval '2 days', 'Point(0.5 0.5 0.5)', '2001-01-15') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint 'Interp=Step;[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]', 2.0, interval '2 days', 'Point(0.5 0.5 0.5)', '2001-01-15') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tpoint) AS tpoint
FROM (SELECT spaceTimeSplit(tgeompoint 'Interp=Step;{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}', 2.0, interval '2 days', 'Point(0.5 0.5 0.5)', '2001-01-15') AS sp) t;

/* Errors */
SELECT spaceTimeSplit(tgeompoint 'SRID=5676;Point(1 1 1)@2001-01-01', 2.0, interval '2 days', 'SRID=3812;Point(0.5 0.5 0.5)');

-------------------------------------------------------------------------------
-- Convenience overloads taking a tgeompoint directly. Each invocation should
-- yield the same set of (index, tile) rows as the explicit
-- @c spaceTiles(stbox(temp), ...) form.
-------------------------------------------------------------------------------

WITH t AS (
  SELECT tgeompoint '[Point(0 0)@2001-01-01, Point(10 10)@2001-01-05]' AS temp
)
SELECT count(*) FROM t, spaceTiles(temp, 2.0);

WITH t AS (
  SELECT tgeompoint '[Point(0 0)@2001-01-01, Point(10 10)@2001-01-05]' AS temp
)
SELECT count(*) FROM t, spaceTiles(temp, 2.0, 4.0);

WITH t AS (
  SELECT tgeompoint '[Point(0 0 0)@2001-01-01, Point(10 10 10)@2001-01-05]' AS temp
)
SELECT count(*) FROM t, spaceTiles(temp, 2.0, 4.0, 5.0);

-- Equivalence: convenience overload vs explicit stbox cast
WITH t AS (
  SELECT tgeompoint '[Point(0 0)@2001-01-01, Point(10 10)@2001-01-05]' AS temp
), a AS (
  SELECT * FROM t, spaceTiles(temp, 2.0)
), b AS (
  SELECT * FROM t, spaceTiles(stbox(temp), 2.0)
)
SELECT bool_and(a.index = b.index AND a.tile ~= b.tile)
FROM a JOIN b ON a.index = b.index;

WITH t AS (
  SELECT tgeompoint '[Point(0 0)@2001-01-01, Point(10 10)@2001-01-05]' AS temp
)
SELECT count(*) FROM t, timeTiles(temp, interval '1 day');

WITH t AS (
  SELECT tgeompoint '[Point(0 0)@2001-01-01, Point(10 10)@2001-01-05]' AS temp
)
SELECT count(*) FROM t, spaceTimeTiles(temp, 2.0, interval '1 day');

WITH t AS (
  SELECT tgeompoint '[Point(0 0)@2001-01-01, Point(10 10)@2001-01-05]' AS temp
)
SELECT count(*) FROM t, spaceTimeTiles(temp, 2.0, 4.0, interval '1 day');

WITH t AS (
  SELECT tgeompoint '[Point(0 0 0)@2001-01-01, Point(10 10 10)@2001-01-05]' AS temp
)
SELECT count(*) FROM t, spaceTimeTiles(temp, 2.0, 4.0, 5.0, interval '1 day');

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- A geodetic point laid on a space grid. A geodetic trip travels the great
-- circle between its positions, which leaves the straight line in longitude
-- and latitude, so the tiles it enters are the tiles that circle crosses.
-------------------------------------------------------------------------------

-- Both ends sit at latitude 61 and the great circle between them reaches
-- latitude 62.485889, so on a grid of 2 degrees the trip enters the row north
-- of the one the straight line in longitude and latitude stays in
SELECT count(DISTINCT ST_Y(s.point::geometry)) AS rows_of_tiles
FROM spaceSplit(tgeogpoint '[Point(10 61)@2001-01-01, Point(50 61)@2001-01-03]',
  2.0) s;
SELECT count(DISTINCT ST_Y(p.point)) AS rows_of_tiles
FROM spaceSplit(tgeompoint
  'SRID=4326;[Point(10 61)@2001-01-01, Point(50 61)@2001-01-03]', 2.0) p;

-- Every fragment is the trip restricted to the fragment's own period
SELECT count(*) AS fragments,
  count(*) FILTER (WHERE atTime(t.tp, getTime(s.tpoint)) <> s.tpoint)
  AS fragments_stating_another_value
FROM (SELECT tgeogpoint
  '[Point(10 61)@2001-01-01, Point(50 61)@2001-01-03]' AS tp) t,
  LATERAL spaceSplit(t.tp, 2.0) s;

-- An arc across the antimeridian takes its short way, so it stays in the tiles
-- of the last and the first columns of the grid
SELECT count(DISTINCT ST_X(s.point::geometry)) AS columns_of_tiles
FROM spaceSplit(tgeogpoint '[Point(170 10)@2001-01-01, Point(-170 -10)@2001-01-02]',
  2.0) s;

-- The space and time grid answers the tile, the time bin and the fragment
SELECT count(*) AS fragments
FROM spaceTimeSplit(tgeogpoint
  '[Point(10 61)@2001-01-01, Point(50 61)@2001-01-03]', 2.0,
  interval '1 day') s;

-- A grid is laid on a value of its own kind, so a geography origin is required
SELECT spaceSplit(tgeogpoint 'Point(1 1)@2001-01-01', 2.0,
  geography 'SRID=4326;Point(0.5 0.5)');

-- Without its upper border a box leaves out only a tile holding nothing of it
-- but that border: a box reaching into a tile keeps it, and a box ending on a
-- tile boundary drops the tile starting there
SELECT borderInc, count(*) AS tiles, array_agg(tile ORDER BY tile) AS tiles
FROM (VALUES (true), (false)) AS b(borderInc),
  LATERAL spaceTiles(stbox 'STBOX X((1,1),(7,3))', 5.0,
    borderInc := b.borderInc) AS tile
GROUP BY borderInc ORDER BY borderInc;
SELECT borderInc, count(*) AS tiles, array_agg(tile ORDER BY tile) AS tiles
FROM (VALUES (true), (false)) AS b(borderInc),
  LATERAL spaceTiles(stbox 'STBOX X((1,1),(10,3))', 5.0,
    borderInc := b.borderInc) AS tile
GROUP BY borderInc ORDER BY borderInc;

-- A trajectory split without the upper border of its extent keeps every tile
-- it passes through, with or without the bit matrix, and loses only the
-- instant it ends on that border
SELECT bitmatrix, borderInc, count(*) AS fragments,
  array_agg(ST_AsText(s.point) ORDER BY ST_AsText(s.point)) AS tiles
FROM (VALUES (true), (false)) AS m(bitmatrix),
  (VALUES (true), (false)) AS b(borderInc),
  LATERAL spaceSplit(tgeompoint
    '[Point(1 1)@2001-01-01, Point(7 3)@2001-01-02, Point(10 2)@2001-01-03]',
    5.0, bitmatrix := m.bitmatrix, borderInc := b.borderInc) s
GROUP BY bitmatrix, borderInc ORDER BY bitmatrix, borderInc;

-- Every form of spaceTiles takes the border by the name borderInc, the one
-- with the three sizes included
SELECT count(*) AS tiles
FROM spaceTiles(stbox 'STBOX Z((1,1,1),(10,3,3))', 5.0, 5.0, 5.0,
  borderInc := false);

-------------------------------------------------------------------------------
