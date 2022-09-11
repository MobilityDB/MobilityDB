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

-- set parallel_tuple_cost=0;
-- set parallel_setup_cost=0;
set force_parallel_mode=regress;

--------------------------------------------------------

SELECT ST_AsText(round(temp::geometry, 6)) FROM tbl_tgeompoint LIMIT 10;
SELECT ST_AsText(round(temp::geometry, 6)) FROM tbl_tgeompoint3D LIMIT 10;

SELECT temp::geometry FROM tbl_tgeompoint LIMIT 10;
SELECT temp::geometry FROM tbl_tgeompoint3D LIMIT 10;

SELECT ST_AsText(round(temp::geography, 6)) FROM tbl_tgeogpoint LIMIT 10;
SELECT ST_AsText(round(temp::geography, 6)) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT temp::geography FROM tbl_tgeogpoint LIMIT 10;
SELECT temp::geography FROM tbl_tgeogpoint3D LIMIT 10;

--------------------------------------------------------

SELECT ST_AsText(round(asGeometry(temp, true), 6)) FROM tbl_tgeompoint LIMIT 10;
SELECT ST_AsText(round(asGeometry(temp, true), 6)) FROM tbl_tgeompoint3D LIMIT 10;

SELECT asGeometry(temp, true) FROM tbl_tgeompoint LIMIT 10;
SELECT asGeometry(temp, true) FROM tbl_tgeompoint3D LIMIT 10;

SELECT ST_AsText(round(asGeography(temp, true), 6)) FROM tbl_tgeogpoint LIMIT 10;
SELECT ST_AsText(round(asGeography(temp, true), 6)) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT asGeography(temp, true) FROM tbl_tgeogpoint LIMIT 10;
SELECT asGeography(temp, true) FROM tbl_tgeogpoint3D LIMIT 10;

-------------------------------------------------------------------------------

SELECT asText(round((temp::geometry)::tgeompoint, 6)) FROM tbl_tgeompoint LIMIT 10;
SELECT asText(round((temp::geometry)::tgeompoint, 6)) FROM tbl_tgeompoint3D LIMIT 10;

SELECT COUNT(*) FROM tbl_tgeompoint WHERE (temp::geometry)::tgeompoint = temp;
SELECT COUNT(*) FROM tbl_tgeompoint3D WHERE (temp::geometry)::tgeompoint = temp;

SELECT asText(round((temp::geography)::tgeogpoint, 6)) FROM tbl_tgeogpoint LIMIT 10;
SELECT asText(round((temp::geography)::tgeogpoint, 6)) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT (temp::geography)::tgeogpoint FROM tbl_tgeogpoint LIMIT 10;
SELECT (temp::geography)::tgeogpoint FROM tbl_tgeogpoint3D LIMIT 10;

-------------------------------------------------------------------------------

SELECT ST_AsText(round(geoMeasure(t1.temp, t2.temp), 6)) FROM tbl_tgeompoint t1, tbl_tfloat t2 WHERE getTime(t1.temp) && getTime(t2.temp);
SELECT ST_AsText(round(geoMeasure(t1.temp, t2.temp), 6)) FROM tbl_tgeompoint3D t1, tbl_tfloat t2 WHERE getTime(t1.temp) && getTime(t2.temp);

SELECT ST_AsText(round(geoMeasure(temp, round(speed(temp),2)), 6)) FROM tbl_tgeompoint WHERE speed(temp) IS NOT NULL ORDER BY k;
SELECT ST_AsText(round(geoMeasure(temp, round(speed(temp),2)), 6)) FROM tbl_tgeompoint3D WHERE speed(temp) IS NOT NULL ORDER BY k;

-------------------------------------------------------------------------------

SELECT MAX(numInstants(simplify(temp, 4))) FROM tbl_tfloat;
SELECT MAX(numInstants(simplify(temp, 4))) FROM tbl_tgeompoint;
SELECT MAX(numInstants(simplify(temp, 4, true))) FROM tbl_tgeompoint;

-------------------------------------------------------------------------------

SELECT round(MAX(ST_Length((mvt).geom))::numeric, 6), MAX(array_length((mvt).times, 1))
FROM (SELECT asMVTGeom(temp, stbox 'STBOX X(((0,0),(50,50)))') AS mvt
  FROM tbl_tgeompoint ) AS t;

-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText((mvt).geom)
-- FROM (SELECT asMVTGeom(tgeompoint '{Point(0 0 0)@2000-01-01, Point(100 100 100)@2000-04-10}',
  -- stbox 'STBOX X(((0,0),(1000,1000)))') AS mvt ) AS t;
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(ST_AsText((mvt).geom))
FROM (SELECT asMVTGeom(tgeompoint '{Point(0 0 0)@2000-01-01, Point(100 100 100)@2000-04-10}',
  stbox 'STBOX X(((0,0),(1000,1000)))') AS mvt ) AS t) AS t(dp);
SELECT ST_AsText((mvt).geom)
FROM (SELECT asMVTGeom(tgeompoint '[Point(0 0)@2000-01-01, Point(100 100)@2000-04-10]',
  stbox 'STBOX X(((40,40),(60,60)))', clip := false) AS mvt ) AS t;
SELECT ST_AsText((mvt).geom)
FROM (SELECT asMVTGeom(tgeompoint '[Point(0 0)@2000-01-01, Point(100 100)@2000-04-10]',
  stbox 'STBOX X(((40,40),(60,60)))') AS mvt ) AS t;
SELECT ST_AsText((mvt).geom)
FROM (SELECT asMVTGeom(tgeompoint '[Point(0 0 0)@2000-01-01, Point(100 100 100)@2000-04-10]',
  stbox 'STBOX X(((40,40),(60,60)))') AS mvt ) AS t;
SELECT ST_AsText((mvt).geom)
FROM (SELECT asMVTGeom(tgeompoint '{[Point(0 0)@2000-01-01], [Point(100 100)@2000-04-10]}',
  stbox 'STBOX X(((0,0),(60,60)))') AS mvt ) AS t;
SELECT ST_AsText((mvt).geom)
FROM (SELECT asMVTGeom(tgeompoint '[Point(0 0)@2000-01-01, Point(0 0)@2000-02-10, Point(100 100)@2000-04-10]',
  stbox 'STBOX X(((0,0),(60,60)))') AS mvt ) AS t;
SELECT ST_AsText((mvt).geom)
FROM (SELECT asMVTGeom(tgeompoint '[Point(0 0)@2000-01-01, Point(100 100)@2000-02-10, Point(100 100)@2000-04-10]',
  stbox 'STBOX X(((0,0),(60,60)))') AS mvt ) AS t;

/* Errors */
SELECT asMVTGeom(tgeompoint '[Point(0 0)@2000-01-01, Point(100 100)@2000-04-10]',
  stbox 'STBOX X(((40,40),(40,40)))');
SELECT asMVTGeom(tgeompoint '[Point(0 0)@2000-01-01, Point(100 100)@2000-04-10]',
  stbox 'STBOX X(((40,40),(60,60)))', 0);

-------------------------------------------------------------------------------

-- set parallel_tuple_cost=100;
-- set parallel_setup_cost=100;
set force_parallel_mode=off;

-------------------------------------------------------------------------------
