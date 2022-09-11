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

--------------------------------------------------------

SELECT ST_AsText(tgeompoint 'Point(1 1)@2000-01-01'::geometry);
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'::geometry);
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'::geometry)) AS t(dp);
SELECT ST_AsText(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'::geometry);
SELECT ST_AsText(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'::geometry);
SELECT ST_AsText(tgeogpoint 'Point(1.5 1.5)@2000-01-01'::geography);
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'::geography);
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints((tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'::geography)::geometry)) AS t(dp);
SELECT ST_AsText(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'::geography);
SELECT ST_AsText(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'::geography);

SELECT ST_AsText(tgeompoint 'Point(1 1 1)@2000-01-01'::geometry);
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'::geometry);
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'::geometry)) AS t(dp);
SELECT ST_AsText(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'::geometry);
SELECT ST_AsText(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'::geometry);
SELECT ST_AsText(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01'::geography);
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'::geography);
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints((tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'::geography)::geometry)) AS t(dp);
SELECT ST_AsText(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'::geography);
SELECT ST_AsText(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'::geography);

-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(tgeompoint 'Interp=Stepwise;[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]'::geometry);
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(tgeompoint 'Interp=Stepwise;[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]'::geometry)) AS t(dp);

--------------------------------------------------------

SELECT ST_AsText(asGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', true));
SELECT ST_AsText(asGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', true));
SELECT ST_AsText(asGeography(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', true));
SELECT ST_AsText(asGeography(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', true));

SELECT ST_AsText(asGeometry(tgeompoint '{[Point(1 1)@2000-01-01]}', true));
select ST_AsText(asGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02],[Point(3 3)@2000-01-03]}', true));

SELECT ST_AsText(asGeometry(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', true));
SELECT ST_AsText(asGeometry(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', true));
SELECT ST_AsText(asGeography(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', true));
SELECT ST_AsText(asGeography(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', true));

--------------------------------------------------------

SELECT asText((tgeompoint 'Point(1 1)@2000-01-01'::geometry)::tgeompoint);
SELECT asText((tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'::geometry)::tgeompoint);
SELECT asText((tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'::geometry)::tgeompoint);
SELECT asText((tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'::geometry)::tgeompoint);
SELECT asText((tgeogpoint 'Point(1.5 1.5)@2000-01-01'::geography)::tgeogpoint);
SELECT asText((tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'::geography)::tgeogpoint);
SELECT asText((tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'::geography)::tgeogpoint);
SELECT asText((tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'::geography)::tgeogpoint);

SELECT asText((tgeompoint 'Point(1 1 1)@2000-01-01'::geometry)::tgeompoint);
SELECT asText((tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'::geometry)::tgeompoint);
SELECT asText((tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'::geometry)::tgeompoint);
SELECT asText((tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'::geometry)::tgeompoint);
SELECT asText((tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01'::geography)::tgeogpoint);
SELECT asText((tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'::geography)::tgeogpoint);
SELECT asText((tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'::geography)::tgeogpoint);
SELECT asText((tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'::geography)::tgeogpoint);

-------------------------------------------------------------------------------

/* Errors */
SELECT geometry 'POINT empty'::tgeompoint;
SELECT geometry 'POINT(1 1)'::tgeompoint;
SELECT geometry 'POLYGON M((1 1 946681200,1 2 946681200,2 2 946681200,2 1 946681200,1 1 946681200))'::tgeompoint;
SELECT geometry 'MULTIPOINT M (1 1 946767600,1 1 946681200)'::tgeompoint;
SELECT geometry 'LINESTRING M (1 1 946767600,1 1 946681200)'::tgeompoint;
SELECT geometry 'GEOMETRYCOLLECTION M (LINESTRING M (1 1 946681200,2 2 946767600), POLYGON M((1 1 946681200,1 2 946681200,2 2 946681200,2 1 946681200,1 1 946681200)))'::tgeompoint;

-------------------------------------------------------------------------------

SELECT ST_AsText(geoMeasure(tgeompoint 'Point(1 1)@2000-01-01', '5@2000-01-01'));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(geoMeasure(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '{5@2000-01-01, 7@2000-01-02, 5@2000-01-03}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(geoMeasure(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '{5@2000-01-01, 7@2000-01-02, 5@2000-01-03}'))) AS t(dp);
SELECT ST_AsText(geoMeasure(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[5@2000-01-01, 7@2000-01-02, 5@2000-01-03]'));
SELECT ST_AsText(geoMeasure(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', '{[5@2000-01-01, 7@2000-01-02, 5@2000-01-03],[5@2000-01-04, 5@2000-01-05]}'));
SELECT ST_AsText(geoMeasure(tgeogpoint 'Point(1.5 1.5)@2000-01-01', '5.5@2000-01-01'));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(geoMeasure(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', '{5.5@2000-01-01, 7.5@2000-01-02, 5@2000-01-03}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(geoMeasure(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', '{5.5@2000-01-01, 7.5@2000-01-02, 5@2000-01-03}')::geometry)) AS t(dp);
SELECT ST_AsText(geoMeasure(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', '[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03]'));
SELECT ST_AsText(geoMeasure(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', '[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03]', true));
SELECT ST_AsText(geoMeasure(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', '{[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03],[5.5@2000-01-04, 5.5@2000-01-05]}'));
SELECT ST_AsText(geoMeasure(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', '{[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03],[5.5@2000-01-04, 5.5@2000-01-05]}', true));

SELECT ST_AsText(geoMeasure(tgeompoint 'Point(1 1 1)@2000-01-01', '5@2000-01-01'));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(geoMeasure(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', '{5@2000-01-01, 7@2000-01-02, 5@2000-01-03}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(geoMeasure(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', '{5@2000-01-01, 7@2000-01-02, 5@2000-01-03}'))) AS t(dp);
SELECT ST_AsText(geoMeasure(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', '[5@2000-01-01, 7@2000-01-02, 5@2000-01-03]'));
SELECT ST_AsText(geoMeasure(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', '{[5@2000-01-01, 7@2000-01-02, 5@2000-01-03],[5@2000-01-04, 5@2000-01-05]}'));
SELECT ST_AsText(geoMeasure(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', '5.5@2000-01-01'));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(geoMeasure(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', '{5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(geoMeasure(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', '{5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03}')::geometry)) AS t(dp);
SELECT ST_AsText(geoMeasure(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', '[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03]'));
SELECT ST_AsText(geoMeasure(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', '{[5.5@2000-01-01, 7.5@2000-01-02, 5.5@2000-01-03],[5.5@2000-01-04, 5.5@2000-01-05]}'));

-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(geoMeasure(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[5@2000-01-01, 7@2000-01-02, 5@2000-01-03]'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(geoMeasure(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[5@2000-01-01, 7@2000-01-02, 5@2000-01-03]'))) AS t(dp);
-- SELECT ST_AsText(geoMeasure(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', '[5@2000-01-01, 7@2000-01-02]'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(geoMeasure(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', '[5@2000-01-01, 7@2000-01-02]'))) AS t(dp);
SELECT ST_AsText(geoMeasure(tgeompoint '[Point(1 1)@2000-01-01]', '[5@2000-01-01]', true));
SELECT ST_AsText(geoMeasure(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[5@2000-01-01, 7@2000-01-02, 5@2000-01-03]', true));
SELECT ST_AsText(geoMeasure(tgeompoint '{[Point(1 1)@2000-01-01]}', '{[5@2000-01-01]}', true));
SELECT ST_AsText(geoMeasure(tgeompoint '{[Point(1 1)@2000-01-01],[Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]}', '{[5@2000-01-01],[6@2000-01-02, 7@2000-01-03]}', true));

-- NULL
SELECT ST_AsText(geoMeasure(tgeompoint 'Point(1 1)@2000-01-01', '5@2000-01-02'));

-------------------------------------------------------------------------------

SELECT numInstants(simplify(tfloat '[4@2000-01-01, 1@2000-01-02, 3@2000-01-03, 1@2000-01-04, 3@2000-01-05, 0@2000-01-06, 4@2000-01-07]', 1));
SELECT numInstants(simplify(tfloat '[4@2000-01-01, 1@2000-01-02, 3@2000-01-03, 1@2000-01-04, 3@2000-01-05, 0@2000-01-06, 4@2000-01-07]', 2));
SELECT numInstants(simplify(tfloat '[4@2000-01-01, 1@2000-01-02, 3@2000-01-03, 1@2000-01-04, 3@2000-01-05, 0@2000-01-06, 4@2000-01-07]', 4));

SELECT simplify(tfloat '{[4@2000-01-01, 1@2000-01-02, 3@2000-01-03, 1@2000-01-04, 3@2000-01-05, 0@2000-01-06, 4@2000-01-07]}', 4);
SELECT simplify(tfloat '{[4@2000-01-01, 1@2000-01-02, 3@2000-01-03, 1@2000-01-04], [3@2000-01-05, 0@2000-01-06, 4@2000-01-07]}', 4);
SELECT simplify(tfloat '{[4@2000-01-01, 1@2000-01-02, 3@2000-01-03, 1@2000-01-04], [3@2000-01-05, 0@2000-01-06, 4@2000-01-07], [4@2000-01-08]}', 4);

-- No simplification, return a copy of the original temporal point
SELECT simplify(tfloat '4@2000-01-01', 1.5);
SELECT simplify(tfloat '{4@2000-01-01, 1@2000-01-02}', 1.5);
SELECT simplify(tfloat '[4@2000-01-01, 1@2000-01-02]', 1.5);
SELECT simplify(tfloat 'Interp=Stepwise;[4@2000-01-01, 1@2000-01-02, 3@2000-01-03]', 1.5);

-- Big temporal point > 256 instants
SELECT numInstants(simplify(tfloat '[47@2000-01-01, 69@2000-01-02, 75@2000-01-03, 77@2000-01-04, 73@2000-01-05, 69@2000-01-06, 70@2000-01-07, 65@2000-01-08, 67@2000-01-09, 61@2000-01-10, 54@2000-01-11, 46@2000-01-12, 41@2000-01-13, 48@2000-01-14, 41@2000-01-15, 44@2000-01-16, 48@2000-01-17, 41@2000-01-18, 33@2000-01-19, 37@2000-01-20, 42@2000-01-21, 41@2000-01-22, 47@2000-01-23, 46@2000-01-24, 52@2000-01-25, 50@2000-01-26, 50@2000-01-27, 48@2000-01-28, 53@2000-01-29, 55@2000-01-30, 47@2000-01-31, 45@2000-02-01, 45@2000-02-02, 53@2000-02-03, 63@2000-02-04, 70@2000-02-05, 63@2000-02-06, 63@2000-02-07, 56@2000-02-08, 60@2000-02-09, 65@2000-02-10, 61@2000-02-11, 68@2000-02-12, 76@2000-02-13, 70@2000-02-14, 79@2000-02-15, 81@2000-02-16, 88@2000-02-17, 81@2000-02-18, 85@2000-02-19, 86@2000-02-20, 94@2000-02-21, 87@2000-02-22, 84@2000-02-23, 80@2000-02-24, 89@2000-02-25, 95@2000-02-26, 89@2000-02-27, 86@2000-02-28, 87@2000-02-29, 85@2000-03-01, 84@2000-03-02, 92@2000-03-03, 83@2000-03-04, 80@2000-03-05, 88@2000-03-06, 94@2000-03-07, 94@2000-03-08, 92@2000-03-09, 83@2000-03-10, 78@2000-03-11, 70@2000-03-12, 65@2000-03-13, 56@2000-03-14, 48@2000-03-15, 46@2000-03-16, 36@2000-03-17, 40@2000-03-18, 35@2000-03-19, 28@2000-03-20, 25@2000-03-21, 32@2000-03-22, 22@2000-03-23, 22@2000-03-24, 17@2000-03-25, 14@2000-03-26, 13@2000-03-27, 10@2000-03-28, 9@2000-03-29, 15@2000-03-30, 16@2000-03-31, 18@2000-04-01, 13@2000-04-02, 12@2000-04-03, 12@2000-04-04, 14@2000-04-05, 11@2000-04-06, 7@2000-04-07,  16@2000-04-08, 21@2000-04-09, 16@2000-04-10, 12@2000-04-11, 20@2000-04-12, 19@2000-04-13, 17@2000-04-14, 26@2000-04-15, 33@2000-04-16, 31@2000-04-17, 34@2000-04-18, 27@2000-04-19, 28@2000-04-20, 38@2000-04-21, 47@2000-04-22, 49@2000-04-23, 49@2000-04-24, 43@2000-04-25, 51@2000-04-26, 60@2000-04-27, 54@2000-04-28, 45@2000-04-29, 55@2000-05-01, 58@2000-05-02, 68@2000-05-03, 62@2000-05-04, 55@2000-05-05, 57@2000-05-06, 58@2000-05-07, 58@2000-05-08, 61@2000-05-09, 57@2000-05-10, 62@2000-05-11, 71@2000-05-12, 65@2000-05-13, 59@2000-05-14, 56@2000-05-15, 49@2000-05-16, 41@2000-05-17, 51@2000-05-19, 46@2000-05-20, 42@2000-05-21, 47@2000-05-22, 42@2000-05-23, 49@2000-05-24, 44@2000-05-25, 43@2000-05-26, 48@2000-05-27, 42@2000-05-28, 45@2000-05-29, 52@2000-05-30, 61@2000-05-31, 59@2000-06-01, 58@2000-06-02, 67@2000-06-03, 69@2000-06-04, 72@2000-06-05, 72@2000-06-06, 58@2000-06-08, 52@2000-06-09, 50@2000-06-10, 58@2000-06-11, 51@2000-06-12, 53@2000-06-13, 46@2000-06-14, 46@2000-06-15, 51@2000-06-16, 46@2000-06-17, 40@2000-06-18, 40@2000-06-19, 41@2000-06-20, 40@2000-06-21, 36@2000-06-22, 41@2000-06-23, 38@2000-06-24, 38@2000-06-25, 33@2000-06-26, 24@2000-06-27, 29@2000-06-28, 45@2000-06-30, 48@2000-07-01, 44@2000-07-02, 41@2000-07-03, 44@2000-07-04, 51@2000-07-05, 42@2000-07-06, 34@2000-07-07, 25@2000-07-08, 18@2000-07-09, 14@2000-07-10, 13@2000-07-11, 5@2000-07-12, 4@2000-07-13, 13@2000-07-14, 8@2000-07-15, 17@2000-07-16, 22@2000-07-17, 22@2000-07-18, 15@2000-07-19, 11@2000-07-20, 2@2000-07-21, 4@2000-07-22, 5@2000-07-23, 11@2000-07-24, 20@2000-07-25, 12@2000-07-26, 2@2000-07-27, 12@2000-07-28, 19@2000-07-29, 35@2000-07-31, 35@2000-08-01, 29@2000-08-02, 25@2000-08-03, 9@2000-08-05, 5@2000-08-06, 10@2000-08-07, 3@2000-08-08, 3@2000-08-10, 4@2000-08-11, 6@2000-08-12, 15@2000-08-13, 17@2000-08-14, 24@2000-08-15, 32@2000-08-16, 30@2000-08-17, 27@2000-08-18, 18@2000-08-19, 20@2000-08-20, 19@2000-08-21, 22@2000-08-22, 15@2000-08-23, 10@2000-08-24, 12@2000-08-25, 7@2000-08-26, 3@2000-08-27, 5@2000-08-28, 14@2000-08-29, 8@2000-08-30, 8@2000-08-31, 10@2000-09-01, 7@2000-09-02, 14@2000-09-03, 17@2000-09-04, 17@2000-09-05, 9@2000-09-06, 18@2000-09-07, 19@2000-09-08, 22@2000-09-09, 20@2000-09-10, 13@2000-09-11, 8@2000-09-12, 6@2000-09-13, 10@2000-09-14, 2@2000-09-15, 7@2000-09-16, 14@2000-09-17, 14@2000-09-18, 10@2000-09-19, 15@2000-09-20, 22@2000-09-21, 31@2000-09-22, 40@2000-09-23, 32@2000-09-24, 33@2000-09-25, 26@2000-09-26, 24@2000-09-27, 12@2000-09-29, 14@2000-09-30, 24@2000-10-02, 34@2000-10-03, 35@2000-10-04, 33@2000-10-06, 36@2000-10-07, 34@2000-10-08, 24@2000-10-09, 21@2000-10-10, 27@2000-10-11, 20@2000-10-12, 21@2000-10-13, 15@2000-10-14, 22@2000-10-15, 26@2000-10-16, 24@2000-10-17, 15@2000-10-18, 7@2000-10-19, 17@2000-10-21, 27@2000-10-22, 31@2000-10-23, 34@2000-10-24, 26@2000-10-25, 22@2000-10-26, 28@2000-10-27, 28@2000-10-28, 27@2000-10-29, 18@2000-10-30, 10@2000-10-31, 4@2000-11-01, 10@2000-11-02, 1@2000-11-03, 6@2000-11-04, 1@2000-11-05, 2@2000-11-06, 0@2000-11-07, 3@2000-11-08, 3@2000-11-09, 9@2000-11-10, 7@2000-11-11, 11@2000-11-12, 7@2000-11-13, 11@2000-11-15, 20@2000-11-16]', 10));

-------------------------------------------------------------------------------

-- Euclidean distance specified
SELECT ST_AsText(trajectory(simplify(tgeompoint '[Point(0 4)@2000-01-01,
  Point(1 1)@2000-01-02, Point(2 3)@2000-01-03, Point(3 1)@2000-01-04,
  Point(4 3)@2000-01-05, Point(5 0)@2000-01-06, Point(6 4)@2000-01-07]', 1.5)));
SELECT ST_AsText(trajectory(simplify(tgeompoint '[Point(0 4)@2000-01-01,
  Point(1 1)@2000-01-02, Point(2 3)@2000-01-03, Point(3 1)@2000-01-04,
  Point(4 3)@2000-01-05, Point(5 0)@2000-01-06, Point(6 4)@2000-01-07]', 4)));

SELECT ST_AsText(trajectory(simplify(tgeompoint '{[Point(0 4)@2000-01-01,
  Point(1 1)@2000-01-02, Point(2 3)@2000-01-03, Point(3 1)@2000-01-04,
  Point(4 3)@2000-01-05, Point(5 0)@2000-01-06, Point(6 4)@2000-01-07]}', 4)));
SELECT ST_AsText(trajectory(simplify(tgeompoint '{[Point(0 4)@2000-01-01,
  Point(1 1)@2000-01-02, Point(2 3)@2000-01-03, Point(3 1)@2000-01-04],
  [Point(4 3)@2000-01-05, Point(5 0)@2000-01-06, Point(6 4)@2000-01-07]}', 4)));
SELECT ST_AsText(trajectory(simplify(tgeompoint '{[Point(0 4)@2000-01-01,
  Point(1 1)@2000-01-02, Point(2 3)@2000-01-03, Point(3 1)@2000-01-04],
  [Point(4 3)@2000-01-05, Point(5 0)@2000-01-06, Point(6 4)@2000-01-07],[Point(7 4)@2000-01-08]}', 4)));

SELECT ST_AsText(trajectory(simplify(tgeompoint '[Point(0 4 0)@2000-01-01,
  Point(1 1 1)@2000-01-02, Point(2 3 2)@2000-01-03, Point(3 1 3)@2000-01-04,
  Point(4 3 4)@2000-01-05, Point(5 0 5)@2000-01-06, Point(6 4 6)@2000-01-07]', 1.5)));
SELECT ST_AsText(trajectory(simplify(tgeompoint '[Point(0 4 0)@2000-01-01,
  Point(1 1 1)@2000-01-02, Point(2 3 2)@2000-01-03, Point(3 1 3)@2000-01-04,
  Point(4 3 4)@2000-01-05, Point(5 0 5)@2000-01-06, Point(6 4 6)@2000-01-07]', 4)));

-- No simplification, return a copy of the original temporal point
SELECT asText(simplify(tgeompoint 'Point(0 4)@2000-01-01', 1.5));
SELECT asText(simplify(tgeompoint '{Point(0 4)@2000-01-01, Point(1 1)@2000-01-02}', 1.5));
SELECT asText(simplify(tgeompoint '[Point(0 4)@2000-01-01, Point(1 1)@2000-01-02]', 1.5));
SELECT asText(simplify(tgeompoint 'Interp=Stepwise;[Point(0 4)@2000-01-01, Point(1 1)@2000-01-02, Point(2 3)@2000-01-03]', 1.5));

-- Coverage of hypot3d
SELECT asText(simplify(tgeompoint '[Point(1 3 5)@2000-01-01, Point(1 4 7)@2000-01-02, Point(1 3 5)@2000-01-03]', 2));
SELECT asText(simplify(tgeompoint '[Point(1 3 5)@2000-01-01, Point(1 3 5)@2000-01-02, Point(1 4 7)@2000-01-03]', 2));

-- Big temporal point > 256 instants
SELECT asText(simplify(tgeompoint '[POINT(77 69)@2000-01-02, POINT(83 75)@2000-01-03, POINT(85 77)@2000-01-04, POINT(82 73)@2000-01-05, POINT(77 69)@2000-01-06, POINT(78 70)@2000-01-07, POINT(73 65)@2000-01-08, POINT(75 67)@2000-01-09, POINT(69 61)@2000-01-10, POINT(62 54)@2000-01-11, POINT(54 46)@2000-01-12, POINT(49 41)@2000-01-13, POINT(57 48)@2000-01-14, POINT(49 41)@2000-01-15, POINT(52 44)@2000-01-16, POINT(56 48)@2000-01-17, POINT(50 41)@2000-01-18, POINT(41 33)@2000-01-19, POINT(45 37)@2000-01-20, POINT(50 42)@2000-01-21, POINT(49 41)@2000-01-22, POINT(55 47)@2000-01-23, POINT(54 46)@2000-01-24, POINT(60 52)@2000-01-25, POINT(58 50)@2000-01-26, POINT(58 50)@2000-01-27, POINT(56 48)@2000-01-28, POINT(62 53)@2000-01-29, POINT(64 55)@2000-01-30, POINT(56 47)@2000-01-31, POINT(53 45)@2000-02-01, POINT(54 45)@2000-02-02, POINT(61 53)@2000-02-03, POINT(71 63)@2000-02-04, POINT(78 70)@2000-02-05, POINT(71 63)@2000-02-06, POINT(72 63)@2000-02-07, POINT(64 56)@2000-02-08, POINT(69 60)@2000-02-09, POINT(73 65)@2000-02-10, POINT(69 61)@2000-02-11, POINT(76 68)@2000-02-12, POINT(85 76)@2000-02-13, POINT(78 70)@2000-02-14, POINT(87 79)@2000-02-15, POINT(89 81)@2000-02-16, POINT(97 88)@2000-02-17, POINT(89 81)@2000-02-18, POINT(93 85)@2000-02-19, POINT(94 86)@2000-02-20, POINT(87 94)@2000-02-21, POINT(80 87)@2000-02-22, POINT(77 84)@2000-02-23, POINT(74 80)@2000-02-24, POINT(83 89)@2000-02-25, POINT(88 95)@2000-02-26, POINT(95 89)@2000-02-27, POINT(92 86)@2000-02-28, POINT(93 87)@2000-02-29, POINT(91 85)@2000-03-01, POINT(90 84)@2000-03-02, POINT(98 92)@2000-03-03, POINT(89 83)@2000-03-04, POINT(86 80)@2000-03-05, POINT(94 88)@2000-03-06, POINT(100 94)@2000-03-07, POINT(100 94)@2000-03-08, POINT(98 92)@2000-03-09, POINT(89 83)@2000-03-10, POINT(84 78)@2000-03-11, POINT(76 70)@2000-03-12, POINT(71 65)@2000-03-13, POINT(62 56)@2000-03-14, POINT(54 48)@2000-03-15, POINT(52 46)@2000-03-16, POINT(42 36)@2000-03-17, POINT(45 40)@2000-03-18, POINT(41 35)@2000-03-19, POINT(34 28)@2000-03-20, POINT(31 25)@2000-03-21, POINT(38 32)@2000-03-22, POINT(28 22)@2000-03-23, POINT(28 22)@2000-03-24, POINT(23 17)@2000-03-25, POINT(20 14)@2000-03-26, POINT(18 13)@2000-03-27, POINT(8 3)@2000-03-28, POINT(2 9)@2000-03-29, POINT(8 15)@2000-03-30, POINT(9 16)@2000-03-31, POINT(10 18)@2000-04-01, POINT(5 13)@2000-04-02, POINT(4 12)@2000-04-03, POINT(5 12)@2000-04-04, POINT(6 14)@2000-04-05, POINT(3 11)@2000-04-06, POINT(7 7)@2000-04-07, POINT(15 16)@2000-04-08, POINT(20 21)@2000-04-09, POINT(15 16)@2000-04-10, POINT(11 12)@2000-04-11, POINT(19 20)@2000-04-12, POINT(18 19)@2000-04-13, POINT(16 17)@2000-04-14, POINT(25 26)@2000-04-15, POINT(32 33)@2000-04-16, POINT(30 31)@2000-04-17, POINT(33 34)@2000-04-18, POINT(26 27)@2000-04-19, POINT(27 28)@2000-04-20, POINT(37 38)@2000-04-21, POINT(46 47)@2000-04-22, POINT(48 49)@2000-04-23, POINT(48 49)@2000-04-24, POINT(42 43)@2000-04-25, POINT(50 51)@2000-04-26, POINT(59 60)@2000-04-27, POINT(53 54)@2000-04-28, POINT(44 45)@2000-04-29, POINT(54 55)@2000-05-01, POINT(57 58)@2000-05-02, POINT(67 68)@2000-05-03, POINT(61 62)@2000-05-04, POINT(54 55)@2000-05-05, POINT(56 57)@2000-05-06, POINT(57 58)@2000-05-07, POINT(57 58)@2000-05-08, POINT(60 61)@2000-05-09, POINT(56 57)@2000-05-10, POINT(61 62)@2000-05-11, POINT(71 71)@2000-05-12, POINT(64 65)@2000-05-13, POINT(59 59)@2000-05-14, POINT(55 56)@2000-05-15, POINT(48 49)@2000-05-16, POINT(40 41)@2000-05-17, POINT(50 51)@2000-05-19, POINT(46 46)@2000-05-20, POINT(41 42)@2000-05-21, POINT(46 47)@2000-05-22, POINT(41 42)@2000-05-23, POINT(48 49)@2000-05-24, POINT(43 44)@2000-05-25, POINT(42 43)@2000-05-26, POINT(47 48)@2000-05-27, POINT(41 42)@2000-05-28, POINT(45 45)@2000-05-29, POINT(51 52)@2000-05-30, POINT(60 61)@2000-05-31, POINT(58 59)@2000-06-01, POINT(58 58)@2000-06-02, POINT(66 67)@2000-06-03, POINT(68 69)@2000-06-04, POINT(71 72)@2000-06-05, POINT(71 72)@2000-06-06, POINT(57 58)@2000-06-08, POINT(51 52)@2000-06-09, POINT(49 50)@2000-06-10, POINT(58 58)@2000-06-11, POINT(51 51)@2000-06-12, POINT(52 53)@2000-06-13, POINT(45 46)@2000-06-14, POINT(45 46)@2000-06-15, POINT(50 51)@2000-06-16, POINT(45 46)@2000-06-17, POINT(39 40)@2000-06-18, POINT(39 40)@2000-06-19, POINT(40 41)@2000-06-20, POINT(40 40)@2000-06-21, POINT(35 36)@2000-06-22, POINT(40 41)@2000-06-23, POINT(37 38)@2000-06-24, POINT(38 38)@2000-06-25, POINT(32 33)@2000-06-26, POINT(23 24)@2000-06-27, POINT(28 29)@2000-06-28, POINT(44 45)@2000-06-30, POINT(47 48)@2000-07-01, POINT(43 44)@2000-07-02, POINT(40 41)@2000-07-03, POINT(43 44)@2000-07-04, POINT(50 51)@2000-07-05, POINT(41 42)@2000-07-06, POINT(33 34)@2000-07-07, POINT(24 25)@2000-07-08, POINT(17 18)@2000-07-09, POINT(13 14)@2000-07-10, POINT(12 13)@2000-07-11, POINT(4 5)@2000-07-12, POINT(3 4)@2000-07-13, POINT(12 13)@2000-07-14, POINT(7 8)@2000-07-15, POINT(16 17)@2000-07-16, POINT(21 22)@2000-07-17, POINT(22 22)@2000-07-18, POINT(14 15)@2000-07-19, POINT(10 11)@2000-07-20, POINT(1 2)@2000-07-21, POINT(3 4)@2000-07-22, POINT(4 5)@2000-07-23, POINT(10 11)@2000-07-24, POINT(19 20)@2000-07-25, POINT(11 12)@2000-07-26, POINT(2 2)@2000-07-27, POINT(11 12)@2000-07-28, POINT(18 19)@2000-07-29, POINT(34 35)@2000-07-31, POINT(34 35)@2000-08-01, POINT(28 29)@2000-08-02, POINT(24 25)@2000-08-03, POINT(8 9)@2000-08-05, POINT(4 5)@2000-08-06, POINT(10 10)@2000-08-07, POINT(2 3)@2000-08-08, POINT(2 3)@2000-08-10, POINT(3 4)@2000-08-11, POINT(5 6)@2000-08-12, POINT(15 15)@2000-08-13, POINT(17 17)@2000-08-14, POINT(24 24)@2000-08-15, POINT(31 32)@2000-08-16, POINT(29 30)@2000-08-17, POINT(26 27)@2000-08-18, POINT(17 18)@2000-08-19, POINT(19 20)@2000-08-20, POINT(18 19)@2000-08-21, POINT(21 22)@2000-08-22, POINT(14 15)@2000-08-23, POINT(9 10)@2000-08-24, POINT(11 12)@2000-08-25, POINT(6 7)@2000-08-26, POINT(2 3)@2000-08-27, POINT(4 5)@2000-08-28, POINT(13 14)@2000-08-29, POINT(7 8)@2000-08-30, POINT(7 8)@2000-08-31, POINT(9 10)@2000-09-01, POINT(6 7)@2000-09-02, POINT(13 14)@2000-09-03, POINT(16 17)@2000-09-04, POINT(16 17)@2000-09-05, POINT(9 9)@2000-09-06, POINT(17 18)@2000-09-07, POINT(18 19)@2000-09-08, POINT(21 22)@2000-09-09, POINT(20 20)@2000-09-10, POINT(12 13)@2000-09-11, POINT(7 8)@2000-09-12, POINT(5 6)@2000-09-13, POINT(10 10)@2000-09-14, POINT(1 2)@2000-09-15, POINT(6 7)@2000-09-16, POINT(14 14)@2000-09-17, POINT(13 14)@2000-09-18, POINT(9 10)@2000-09-19, POINT(14 15)@2000-09-20, POINT(21 22)@2000-09-21, POINT(31 31)@2000-09-22, POINT(39 40)@2000-09-23, POINT(31 32)@2000-09-24, POINT(32 33)@2000-09-25, POINT(25 26)@2000-09-26, POINT(23 24)@2000-09-27, POINT(11 12)@2000-09-29, POINT(13 14)@2000-09-30, POINT(23 24)@2000-10-02, POINT(33 34)@2000-10-03, POINT(34 35)@2000-10-04, POINT(32 33)@2000-10-06, POINT(36 36)@2000-10-07, POINT(33 34)@2000-10-08, POINT(23 24)@2000-10-09, POINT(20 21)@2000-10-10, POINT(26 27)@2000-10-11, POINT(19 20)@2000-10-12, POINT(20 21)@2000-10-13, POINT(14 15)@2000-10-14, POINT(22 22)@2000-10-15, POINT(25 26)@2000-10-16, POINT(24 24)@2000-10-17, POINT(14 15)@2000-10-18, POINT(6 7)@2000-10-19, POINT(16 17)@2000-10-21, POINT(26 27)@2000-10-22, POINT(30 31)@2000-10-23, POINT(33 34)@2000-10-24, POINT(25 26)@2000-10-25, POINT(21 22)@2000-10-26, POINT(27 28)@2000-10-27, POINT(27 28)@2000-10-28, POINT(27 27)@2000-10-29, POINT(17 18)@2000-10-30, POINT(9 10)@2000-10-31, POINT(3 4)@2000-11-01, POINT(9 10)@2000-11-02, POINT(0 1)@2000-11-03, POINT(5 6)@2000-11-04, POINT(0 1)@2000-11-05, POINT(1 2)@2000-11-06, POINT(2 0)@2000-11-07, POINT(5 3)@2000-11-08, POINT(6 3)@2000-11-09, POINT(11 9)@2000-11-10, POINT(9 7)@2000-11-11, POINT(13 11)@2000-11-12, POINT(9 7)@2000-11-13, POINT(13 11)@2000-11-15, POINT(22 20)@2000-11-16]', 10));

-------------------------------------------------------------------------------

-- Standard vs Synchronized Euclidean distance

SELECT asText(simplify(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-04]', 1));
SELECT asText(simplify(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-04]', 1, true));

-------------------------------------------------------------------------------
