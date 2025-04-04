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
-- Multidimensional tiling
-------------------------------------------------------------------------------

SELECT spaceTiles(tgeometry '[Point(3 3)@2000-01-15, Point(15 15)@2000-01-25]'::stbox, 2.0) LIMIT 3;
SELECT spaceTiles(tgeometry 'SRID=3812;[Point(3 3)@2000-01-15, Point(15 15)@2000-01-25]'::stbox, 2.0, geometry 'Point(3 3)') LIMIT 3;
SELECT spaceTiles(tgeometry '[Point(3 3 3)@2000-01-15, Point(15 15 15)@2000-01-25]'::stbox, 2.0, geometry 'Point(3 3 3)') LIMIT 3;
SELECT spaceTimeTiles(tgeometry '[Point(3 3)@2000-01-15, Point(15 15)@2000-01-25]'::stbox, 2.0, interval '2 days', 'Point(3 3)', '2000-01-15') LIMIT 3;
SELECT spaceTimeTiles(tgeometry '[Point(3 3 3)@2000-01-15, Point(15 15 15)@2000-01-25]'::stbox, 2.0, interval '2 days', 'Point(3 3 3)', '2000-01-15') LIMIT 3;
/* Errors */
SELECT spaceTiles(tgeometry '[Point(3 3 3)@2000-01-15, Point(15 15 15)@2000-01-25]'::stbox, 2.0, geometry 'Point(3 3)');
SELECT spaceTiles(tgeometry 'SRID=3812;[Point(3 3)@2000-01-15, Point(15 15)@2000-01-25]'::stbox, 2.0, geometry 'SRID=5676;Point(1 1)');
SELECT spaceTiles(tgeography '[Point(3 3)@2000-01-15, Point(15 15)@2000-01-25]'::stbox, 2.0);

SELECT getSpaceTile(geometry 'Point(3 3)', 2.0);
SELECT getSpaceTile(geometry 'Point(3 3 3)', 2.0);
SELECT getStboxTimeTile(timestamptz '2000-01-15', interval '2 days');
SELECT getStboxTimeTile(timestamptz '2000-01-15', interval '2 days', '2020-06-15');
SELECT getSpaceTimeTile(geometry 'Point(3 3)', timestamptz '2000-01-15', 2.0, interval '2 days');
SELECT getSpaceTimeTile(geometry 'Point(3 3)', timestamptz '2000-01-15', 2.0, interval '2 days');
SELECT getSpaceTimeTile(geometry 'Point(3 3 3)', timestamptz '2000-01-15', 2.0, interval '2 days', geometry 'Point(1 1 1)', '2020-06-15');

SELECT getSpaceTimeTile(geometry 'SRID=3812;Point(3 3 3)', timestamptz '2000-01-15', 2.0, interval '2 days', geometry 'SRID=3812;Point(1 1 1)', '2020-06-15');
/* Errors */
SELECT getSpaceTimeTile(geometry 'Point(3 3 3)', timestamptz '2000-01-15', 2.0, interval '2 days', geometry 'Point(1 1)', '2020-06-15');
SELECT getSpaceTimeTile(geometry 'SRID=3812;Point(3 3 3)', timestamptz '2000-01-15', 2.0, interval '2 days', geometry 'SRID=2154;Point(1 1)', '2020-06-15');

-------------------------------------------------------------------------------
-- Space boxes
-------------------------------------------------------------------------------

SELECT round(spaceBoxes(tgeometry '[Point(1 1)@2000-01-01, Point(10 10)@2000-01-10]', 2.0), 6);
SELECT round(spaceBoxes(tgeometry 'SRID=3812;[Point(1 1)@2000-01-01, Point(10 10)@2000-01-10]', 2.0, geometry 'Point(1 1)'), 6);

/* Errors */
SELECT spaceBoxes(tgeometry '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-10]', 2.0, geometry 'Point(1 1)');
SELECT spaceBoxes(tgeometry 'SRID=3812;[Point(1 1)@2000-01-01, Point(10 10)@2000-01-10]', 2.0, geometry 'SRID=5676;Point(1 1)');

-------------------------------------------------------------------------------
-- time boxes
-------------------------------------------------------------------------------

SELECT round(timeBoxes(tgeometry '[Point(1 1)@2000-01-01, Point(10 10)@2000-01-10]', interval '2 days', '2000-01-01'), 6);

/* Errors */
SELECT round(timeBoxes(tgeometry '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-10]', interval '2 days', '2000-01-01'));SELECT timeBoxes(tgeometry '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-10]', interval '2 months', '2000-01-01');

-------------------------------------------------------------------------------
-- SpaceTime boxes
-------------------------------------------------------------------------------

SELECT round(spaceTimeBoxes(tgeometry '[Point(1 1)@2000-01-01, Point(10 10)@2000-01-10]', 2.0, interval '2 days', 'Point(1 1)', '2000-01-01'), 6);

/* Errors */
SELECT round(spaceTimeBoxes(tgeometry '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-10]', 2.0, interval '2 days', 'Point(1 1 1)', '2000-01-01'));
SELECT spaceTimeBoxes(tgeometry '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-10]', 2.0, interval '2 days', 'Point(1 1)', '2000-01-01');
SELECT spaceTimeBoxes(tgeometry 'SRID=3812;[Point(3 3)@2000-01-15, Point(15 15)@2000-01-25]', 2.0, interval '2 days', 'SRID=5676;Point(1 1)', '2000-01-01');

-------------------------------------------------------------------------------
-- Space split
-------------------------------------------------------------------------------

-- 2D
SELECT ST_AsText((sp).point) AS point, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceSplit(tgeometry 'Point(1 1)@2000-01-01', 2.0) AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceSplit(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2.0) AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceSplit(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2.0) AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceSplit(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2.0) AS sp) t;

SELECT ST_AsText((sp).point) AS point, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceSplit(tgeometry 'Point(1 1)@2000-01-01', 2.0, geometry 'Point(0.5 0.5)') AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceSplit(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2.0, geometry 'Point(0.5 0.5)') AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceSplit(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2.0, geometry 'Point(0.5 0.5)') AS sp) t;
SELECT ST_AsText((sp).point) AS point, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceSplit(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2.0, geometry 'Point(0.5 0.5)') AS sp) t;

/* Errors */
SELECT spaceSplit(tgeometry 'SRID=5676;Point(1 1 1)@2000-01-01', 2.0, geometry 'SRID=3812;Point(0.5 0.5 0.5)');

-------------------------------------------------------------------------------
-- Space-time split
-------------------------------------------------------------------------------

-- Without bitmatrix
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceTimeSplit(tgeometry 'Point(1 1)@2000-01-01', 2.0, interval '2 days', bitmatrix:=false) AS sp) t;

-- 2D
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceTimeSplit(tgeometry 'Point(1 1)@2000-01-01', 2.0, interval '2 days') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceTimeSplit(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2.0, interval '2 days') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceTimeSplit(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2.0, interval '2 days') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceTimeSplit(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2.0, interval '2 days') AS sp) t;

SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceTimeSplit(tgeometry 'Point(1 1)@2000-01-01', 2.0, interval '2 days', 'Point(0.5 0.5)', '2000-01-15') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceTimeSplit(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2.0, interval '2 days', 'Point(0.5 0.5)', '2000-01-15') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceTimeSplit(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2.0, interval '2 days', 'Point(0.5 0.5)', '2000-01-15') AS sp) t;
SELECT ST_AsText((sp).point) AS point, (sp).time, astext((sp).tgeo) AS tgeo
FROM (SELECT spaceTimeSplit(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2.0, interval '2 days', 'Point(0.5 0.5)', '2000-01-15') AS sp) t;

/* Errors */
SELECT spaceTimeSplit(tgeometry 'SRID=5676;Point(1 1 1)@2000-01-01', 2.0, interval '2 days', 'SRID=3812;Point(0.5 0.5 0.5)');

-------------------------------------------------------------------------------
