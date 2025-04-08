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
-- Conversions

SELECT asText(tgeompoint(tgeometry 'Point(1 1)@2000-01-01'));
SELECT asText(tgeompoint(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asText(tgeompoint(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asText(tgeompoint(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asText(tgeogpoint(tgeography 'Point(1.5 1.5)@2000-01-01'));
SELECT asText(tgeogpoint(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asText(tgeogpoint(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asText(tgeogpoint(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
/* Errors */
SELECT tgeompoint(tgeometry 'Linestring(1 1,2 2)@2000-01-01');
SELECT tgeompoint(tgeometry '{Linestring(1 1,2 2)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tgeompoint(tgeometry '[Linestring(1 1,2 2)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tgeompoint(tgeometry '{[Linestring(1 1,2 2)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

-------------------------------------------------------------------------------

SELECT asText(tgeometry(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asText(tgeometry(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asText(tgeometry(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asText(tgeometry(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asText(tgeography(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT asText(tgeography(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asText(tgeography(tgeogpoint 'Interp=Step;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asText(tgeography(tgeogpoint 'Interp=Step;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
/* Errors */
SELECT tgeography(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT tgeography(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
SELECT tgeompoint(tgeometry 'Linestring(1 1,2 2)@2000-01-01');
SELECT tgeompoint(tgeometry '{Linestring(1 1,2 2)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tgeompoint(tgeometry 'Interp=Step;[Linestring(1 1,2 2)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tgeompoint(tgeometry 'Interp=Step;{[Linestring(1 1,2 2)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

-------------------------------------------------------------------------------
-- Geoset

SELECT SRID(geomset '{"SRID=5676;Point(1 1)", "SRID=5676;Point(2 2)"}');
SELECT asEWKT(setSRID(geomset '{"Point(0 0)", "Point(1 1)"}', 5676));

-- Tests independent of the PROJ version
WITH test(geoset) AS (
  SELECT geomset 'SRID=4326;{"Point(1.0 2.0 3.0)","Point(4.0 5.0 6.0)"}' )
SELECT asEWKT(transform(transform(geoset, 5676), 4326), 6) FROM test;

-- Noop
SELECT asEWKT(transform(geomset 'SRID=4326;{"Point(1.0 2.0 3.0)","Point(4.0 5.0 6.0)"}', 4326));

-- Transform to ESRI:102004
WITH test(geoset) AS (
  SELECT geomset 'SRID=4326;{"Point(1.0 2.0 3.0)","Point(4.0 5.0 6.0)"}' )
SELECT asEWKT(transform(transform(geoset, 102004), 4326), 6) FROM test;

-- Coverage for erroneous from/to SRID
WITH test(geoset) AS (
  SELECT geomset 'SRID=909090;{"Point(1.0 2.0 3.0)","Point(4.0 5.0 6.0)"}' )
SELECT asEWKT(transform(transform(geoset, 102004), 909090), 6) FROM test;
WITH test(geoset) AS (
  SELECT geomset 'SRID=4326;{"Point(1.0 2.0 3.0)","Point(4.0 5.0 6.0)"}' )
SELECT asEWKT(transform(transform(geoset, 909090), 4326), 6) FROM test;

-------------------------------------------------------------------------------
-- STBOX

SELECT SRID(stbox 'STBOX ZT(((1.0,2.0,3.0),(4.0,5.0,6.0)),[2000-01-01,2000-01-02])');
SELECT SRID(stbox 'SRID=4326;STBOX ZT(((1.0,2.0,3.0),(4.0,5.0,6.0)),[2000-01-01,2000-01-02])');
/* Errors */
SELECT SRID(stbox 'STBOX T([2000-01-01,2000-01-02])');

SELECT setSRID(stbox 'STBOX X((1,1),(2,2))', 5676);
SELECT setSRID(stbox 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])', 5676);
SELECT setSRID(stbox 'STBOX Z((1,1,1),(2,2,2))', 5676);
SELECT setSRID(stbox 'STBOX ZT(((1,1,1),(2,2,2)),[2000-01-01,2000-01-02])', 5676);
SELECT setSRID(stbox 'GEODSTBOX Z((1,1,1),(2,2,2))', 4326);
SELECT setSRID(stbox 'GEODSTBOX ZT(((1,1,1),(2,2,2)),[2000-01-01,2000-01-02])', 4326);
/* Errors */
SELECT setSRID(stbox 'STBOX T([2000-01-01,2000-01-02])', 5676);
SELECT setSRID(stbox 'GEODSTBOX T([2000-01-01,2000-01-02])', 4326);

-- Robustness tests
SELECT round(transform(transform(stbox 'SRID=4326;STBOX X((1,1),(2,2))', 5676), 4326), 1);
SELECT round(transform(transform(stbox 'SRID=4326;STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])', 5676), 4326), 1);
SELECT round(transform(transform(stbox 'SRID=4326;STBOX Z((1,1,1),(2,2,2))', 5676), 4326), 1);
SELECT round(transform(transform(stbox 'SRID=4326;STBOX ZT(((1,1,1),(2,2,2)),[2000-01-01,2000-01-02])', 5676), 4326), 1);
SELECT round(transform(transform(stbox 'SRID=4326;GEODSTBOX Z((1,1,1),(2,2,2))', 4269), 4326), 1);
SELECT round(transform(transform(stbox 'SRID=4326;GEODSTBOX ZT(((1,1,1),(2,2,2)),[2000-01-01,2000-01-02])', 4269), 4326), 1);

SELECT DISTINCT SRID(b) FROM tbl_stbox;
SELECT MIN(xmin(setSRID(b,4326))) FROM tbl_stbox;

SELECT ROUND(MIN(xmin(transform(transform(setSRID(b,5676), 4326), 5676))), 1) FROM tbl_stbox;
SELECT MIN(xmin(round(transform(transform(setSRID(b,5676), 4326), 5676), 1))) FROM tbl_stbox;

-- Noop
SELECT transform(stbox 'SRID=4326;STBOX X((1,1),(2,2))', 4326);

/* Errors */
SELECT transform(stbox 'SRID=4326;STBOX X((1,1),(2,2))', 0);

-------------------------------------------------------------------------------
-- 2D
SELECT SRID(tgeometry 'Point(1 1)@2000-01-01');
SELECT SRID(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT SRID(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT SRID(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT SRID(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT SRID(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT SRID(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT SRID(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
-- 3D
SELECT SRID(tgeometry 'Point(1 1 1)@2000-01-01');
SELECT SRID(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT SRID(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT SRID(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT SRID(tgeography 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT SRID(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT SRID(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT SRID(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

SELECT asEWKT(setSRID(tgeometry 'Point(1 1 1)@2000-01-01', 5676));
SELECT asEWKT(setSRID(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 5676));
SELECT asEWKT(setSRID(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 5676));
SELECT asEWKT(setSRID(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 5676));
SELECT asEWKT(setSRID(tgeography 'Point(1.5 1.5 1.5)@2000-01-01', 4269));
SELECT asEWKT(setSRID(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 4269));
SELECT asEWKT(setSRID(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 4269));
SELECT asEWKT(setSRID(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 4269));

SELECT startValue(transform(tgeometry 'SRID=5676;Point(1 2 3)@2000-01-01', 4326)) = st_transform(geometry 'SRID=5676;Point(1 2 3)', 4326);
SELECT startValue(transform(tgeometry 'SRID=5676;{Point(1 2 3)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 2 3)@2000-01-03}', 4326)) = st_transform(geometry 'SRID=5676;Point(1 2 3)', 4326);
SELECT startValue(transform(tgeometry 'SRID=5676;[Point(1 2 3)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 2 3)@2000-01-03]', 4326)) = st_transform(geometry 'SRID=5676;Point(1 2 3)', 4326);
SELECT startValue(transform(tgeometry 'SRID=5676;{[Point(1 2 3)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 2 3)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 4326)) = st_transform(geometry 'SRID=5676;Point(1 2 3)', 4326);
-- Noop
SELECT startValue(transform(tgeometry 'SRID=5676;Point(1 2 3)@2000-01-01', 5676)) = st_transform(geometry 'SRID=5676;Point(1 2 3)', 5676);
SELECT startValue(transform(tgeometry 'SRID=5676;Point(1 2 3)@2000-01-01', 4326)) = st_transform(geometry 'SRID=4326;Point(1 2 3)', 4326);

-------------------------------------------------------------------------------
-- Transform with pipeline

WITH test(geoset, pipeline) AS (
  SELECT geomset 'SRID=4326;{"Point(1.0 2.0 3.0)","Point(4.0 5.0 6.0)"}',
   text 'urn:ogc:def:coordinateOperation:EPSG::16031' )
SELECT asText(transformPipeline(transformPipeline(geoset, pipeline, 4326), pipeline, 4326, false), 3) = asText(geoset, 3)
FROM test;

WITH test(box, pipeline) AS (
  SELECT tgeometry 'SRID=4326;[POINT(2.123456 49.123456)@2000-01-01, POINT(3.123456 50.123456)@2000-01-02]'::stbox,
   text 'urn:ogc:def:coordinateOperation:EPSG::16031' )
SELECT asText(transformPipeline(transformPipeline(box, pipeline, 4326), pipeline, 4326, false), 3) = asText(box, 3)
FROM test;

-- This test ensure correctness between PostGIS and MobilityDB results.
-- However, it was commented out since not all installations have the latest
-- PostGIS version 3.4
-- SELECT startValue(transformPipeline(tgeometry 'SRID=4326;Point(1 2 3)@2000-01-01',
  -- 'urn:ogc:def:coordinateOperation:EPSG::16031', 5676, true)) =
  -- st_transformpipeline(geometry 'SRID=4326;Point(1 2 3)', 'urn:ogc:def:coordinateOperation:EPSG::16031', 5676);

SELECT asText(transformPipeline(tgeometry 'SRID=4326;POINT(2 49)@2000-01-01',
  text 'urn:ogc:def:coordinateOperation:EPSG::16031'));
SELECT asText(transformPipeline(tgeometry 'POINT(426857.9877165967 5427937.523342293)@2000-01-01 00:00:00+01',
  text 'urn:ogc:def:coordinateOperation:EPSG::16031', 4326, false), 3);

WITH test1(pipeline) AS (
  SELECT text 'urn:ogc:def:coordinateOperation:EPSG::16031' ),
test2(temp) AS (
  SELECT tgeometry 'SRID=4326;POINT(2.123456 49.123456)@2000-01-01' UNION
  SELECT tgeometry 'SRID=4326;{POINT(2.123456 49.123456)@2000-01-01, POINT(3.123456 50.123456)@2000-01-02}' UNION
  SELECT tgeometry 'SRID=4326;[POINT(2.123456 49.123456)@2000-01-01, POINT(3.123456 50.123456)@2000-01-02]' UNION
  SELECT tgeometry 'SRID=4326;{[POINT(2.123456 49.123456)@2000-01-01, POINT(3.123456 50.123456)@2000-01-02],'
    '[POINT(2.123456 49.123456)@2000-01-03, POINT(3.123456 50.123456)@2000-01-04]}'
    )
SELECT DISTINCT asText(transformPipeline(transformPipeline(temp, pipeline, 4326), pipeline, 4326, false), 3) = asText(temp, 3)
FROM test1, test2;

--------------------------------------------------------

-- 2D
SELECT ST_AsText(round(geometry 'Point(1.123456789 1.123456789)', 6));
SELECT ST_AsText(round(geometry 'Linestring(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789)', 6));
SELECT ST_AsText(round(geometry 'Triangle((1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 1.123456789,1.123456789 1.123456789))', 6));
SELECT ST_AsText(round(geometry 'Circularstring(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789)', 6));
SELECT ST_AsText(round(geometry 'CompoundCurve(Circularstring(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789),(3.123456789 3.123456789, 4.123456789 4.123456789))', 6));
SELECT ST_AsText(round(geometry 'Multicurve(Circularstring(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789),(3.123456789 3.123456789,2.123456789 2.123456789,1.123456789 1.123456789),CompoundCurve(Circularstring(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789),(3.123456789 3.123456789, 4.123456789 4.123456789)))', 6));
SELECT ST_AsText(round(geometry 'Polygon((1.123456789 1.123456789,4.123456789 4.123456789,7.123456789 1.123456789,1.123456789 1.123456789),(3.123456789 2.123456789,4.123456789 3.123456789,5.123456789 2.123456789,3.123456789 2.123456789))', 6));
SELECT ST_AsText(round(geometry 'CurvePolygon(Circularstring(1.123456789 1.123456789,2.123456789 2.123456789,1.123456789 1.123456789),(2.123456789 2.123456789,1.123456789 2.123456789,2.123456789 1.123456789,2.123456789 2.123456789))', 6));
SELECT ST_AsText(round(geometry 'CurvePolygon(CompoundCurve(Circularstring(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789),(3.123456789 3.123456789, 4.123456789 4.123456789,1.123456789 1.123456789)))', 6));
-- PostGIS 3.3 changed the output of MULTIPOINT
SELECT ST_NPoints(round(geometry 'MultiPoint((1.123456789 1.123456789),(2.123456789 2.123456789),(3.123456789 3.123456789))', 6));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(round(geometry 'MultiPoint((1.123456789 1.123456789),(2.123456789 2.123456789),(3.123456789 3.123456789))', 6))) AS t(dp);
SELECT ST_AsText(round(geometry 'MultiLinestring((1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789),(4.123456789 4.123456789,5.123456789 5.123456789))', 6));
SELECT ST_AsText(round(geometry 'MultiPolygon(((1.123456789 1.123456789,4.123456789 4.123456789,7.123456789 1.123456789,1.123456789 1.123456789)),((3.123456789 2.123456789,4.123456789 3.123456789,5.123456789 2.123456789,3.123456789 2.123456789)))', 6));
SELECT ST_AsText(round(geometry 'GeometryCollection(Point(1.123456789 1.123456789),
  Linestring(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789),
  Triangle((1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 1.123456789,1.123456789 1.123456789)),
  Circularstring(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789),
  Polygon((1.123456789 1.123456789,4.123456789 4.123456789,7.123456789 1.123456789,1.123456789 1.123456789),(3.123456789 2.123456789,4.123456789 3.123456789,5.123456789 2.123456789,3.123456789 2.123456789)))', 6));
SELECT ST_AsText(round(geometry 'GeometryCollection(
  MultiPoint(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789),
  MultiLinestring((1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789),(4.123456789 4.123456789,5.123456789 5.123456789)),
  MultiPolygon(((1.123456789 1.123456789,4.123456789 4.123456789,7.123456789 1.123456789,1.123456789 1.123456789)),((3.123456789 2.123456789,4.123456789 3.123456789,5.123456789 2.123456789,3.123456789 2.123456789))))', 6));
SELECT ST_AsText(round(geometry 'GeometryCollection(
  Multicurve(Circularstring(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789),(3.123456789 3.123456789,2.123456789 2.123456789,1.123456789 1.123456789)), 
  CompoundCurve(Circularstring(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789),(3.123456789 3.123456789, 4.123456789 4.123456789)), 
  CurvePolygon(Circularstring(1.123456789 1.123456789,2.123456789 2.123456789,1.123456789 1.123456789)))', 6));

SELECT ST_AsText(round(geometry 'Point Empty', 6));
SELECT ST_AsText(round(geometry 'Linestring Empty', 6));
SELECT ST_AsText(round(geometry 'Triangle Empty', 6));
SELECT ST_AsText(round(geometry 'Circularstring Empty', 6));
SELECT ST_AsText(round(geometry 'CompoundCurve Empty', 6));
SELECT ST_AsText(round(geometry 'Polygon Empty', 6));
SELECT ST_AsText(round(geometry 'CurvePolygon Empty', 6));
SELECT ST_AsText(round(geometry 'MultiPoint Empty', 6));
SELECT ST_AsText(round(geometry 'MultiLinestring Empty', 6));
SELECT ST_AsText(round(geometry 'MultiPolygon Empty', 6));
-- 3D
SELECT ST_AsText(round(geometry 'Point Z(1.123456789 1.123456789 1.123456789)', 6));
SELECT ST_AsText(round(geometry 'Linestring Z(1.123456789 1.123456789 1.123456789,2.123456789 2.123456789 2.123456789,3.123456789 3.123456789 3.123456789)', 6));
SELECT ST_AsText(round(geometry 'Triangle Z((1.123456789 1.123456789 1.123456789,2.123456789 2.123456789 2.123456789,3.123456789 1.123456789 1.123456789,1.123456789 1.123456789 1.123456789))', 6));
SELECT ST_AsText(round(geometry 'Circularstring Z(1.123456789 1.123456789 1.123456789,2.123456789 2.123456789 2.123456789,3.123456789 3.123456789 3.123456789)', 6));
SELECT ST_AsText(round(geometry 'Polygon Z((1.123456789 1.123456789 1.123456789,4.123456789 4.123456789 4.123456789,7.123456789 1.123456789 1.123456789,1.123456789 1.123456789 1.123456789),(3.123456789 2.123456789 2.123456789,4.123456789 3.123456789 3.123456789,5.123456789 2.123456789 2.123456789,3.123456789 2.123456789 2.123456789))', 6));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(round(geometry 'MultiPoint Z(1.123456789 1.123456789 1.123456789,2.123456789 2.123456789 2.123456789,3.123456789 3.123456789 3.123456789)', 6));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(round(geometry 'MultiPoint Z(1.123456789 1.123456789 1.123456789,2.123456789 2.123456789 2.123456789,3.123456789 3.123456789 3.123456789)', 6))) AS t(dp);
SELECT ST_AsText(round(geometry 'MultiLinestring Z((1.123456789 1.123456789 1.123456789,2.123456789 2.123456789 2.123456789,3.123456789 3.123456789 3.123456789),(4.123456789 4.123456789 4.123456789,5.123456789 5.123456789 5.123456789))', 6));
SELECT ST_AsText(round(geometry 'MultiPolygon Z(((1.123456789 1.123456789 1.123456789,4.123456789 4.123456789 4.123456789,7.123456789 1.123456789 1.123456789,1.123456789 1.123456789 1.123456789)),((3.123456789 2.123456789 2.123456789,4.123456789 3.123456789 3.123456789,5.123456789 2.123456789 2.123456789,3.123456789 2.123456789 2.123456789)))', 6));

/* Errors */
SELECT round(geometry '
POLYHEDRALSURFACE Z (
  ((0 0 0, 0 1 0, 1 1 0, 1 0 0, 0 0 0)),
  ((0 0 0, 0 1 0, 0 1 1, 0 0 1, 0 0 0)),
  ((0 0 0, 1 0 0, 1 0 1, 0 0 1, 0 0 0)),
  ((1 1 1, 1 0 1, 0 0 1, 0 1 1, 1 1 1)),
  ((1 1 1, 1 0 1, 1 0 0, 1 1 0, 1 1 1)),
  ((1 1 1, 1 1 0, 0 1 0, 0 1 1, 1 1 1))
)', 1);
SELECT round(geometry 'GeometryCollection(
Point(1 1 1),
POLYHEDRALSURFACE Z (
  ((0 0 0, 0 1 0, 1 1 0, 1 0 0, 0 0 0)),
  ((0 0 0, 0 1 0, 0 1 1, 0 0 1, 0 0 0)),
  ((0 0 0, 1 0 0, 1 0 1, 0 0 1, 0 0 0)),
  ((1 1 1, 1 0 1, 0 0 1, 0 1 1, 1 1 1)),
  ((1 1 1, 1 0 1, 1 0 0, 1 1 0, 1 1 1)),
  ((1 1 1, 1 1 0, 0 1 0, 0 1 1, 1 1 1))
))', 1);

--------------------------------------------------------

-- 2D
SELECT asText(round(tgeometry 'Point(1.12345 1.12345)@2000-01-01', 2));
SELECT asText(round(tgeometry '{Point(1.12345 1.12345)@2000-01-01, Point(2 2)@2000-01-02, Point(1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(round(tgeometry '[Point(1.12345 1.12345)@2000-01-01, Point(2 2)@2000-01-02, Point(1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(round(tgeometry '{[Point(1.12345 1.12345)@2000-01-01, Point(2 2)@2000-01-02, Point(1.12345 1.12345)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2));
SELECT asText(round(tgeography 'Point(1.12345 1.12345)@2000-01-01', 2));
SELECT asText(round(tgeography '{Point(1.12345 1.12345)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(round(tgeography '[Point(1.12345 1.12345)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(round(tgeography '{[Point(1.12345 1.12345)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.12345 1.12345)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2));
-- 3D
SELECT asText(round(tgeometry 'Point(1.12345 1.12345 1.12345)@2000-01-01', 2));
SELECT asText(round(tgeometry '{Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(round(tgeometry '[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(round(tgeometry '{[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2));
SELECT asText(round(tgeography 'Point(1.12345 1.12345 1.12345)@2000-01-01', 2));
SELECT asText(round(tgeography '{Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(round(tgeography '[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(round(tgeography '{[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2));

SELECT asText(round(ARRAY[tgeometry '[Point(1.55 1.55)@2000-01-01, Point(2.55 2.55)@2000-01-02, Point(1.55 1.55)@2000-01-03]', '[Point(3.55 3.55)@2000-01-04, Point(3.55 3.55)@2000-01-05]'],1));
SELECT round(ARRAY[]::tgeometry[]);

--------------------------------------------------------

SELECT asEWKT(tgeometry 'Point(1 1)@2000-01-01'::tgeography);
SELECT asEWKT(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'::tgeography);
SELECT asEWKT(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'::tgeography);
SELECT asEWKT(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'::tgeography);

SELECT asEWKT(tgeography 'Point(1.5 1.5)@2000-01-01'::tgeometry);
SELECT asEWKT(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'::tgeometry);
SELECT asEWKT(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'::tgeometry);
SELECT asEWKT(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'::tgeometry);

--------------------------------------------------------

-- 2D
SELECT ST_AsText(traversedArea(tgeometry 'Point(1 1)@2000-01-01'));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(traversedArea(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(traversedArea(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'))) AS t(dp);
SELECT ST_AsText(traversedArea(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsText(traversedArea(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsText(traversedArea(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsText(traversedArea(tgeography 'Point(1.5 1.5)@2000-01-01'));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(traversedArea(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(traversedArea(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}')::geometry)) AS t(dp);
SELECT ST_AsText(traversedArea(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT ST_AsText(traversedArea(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
-- 3D
SELECT ST_AsText(traversedArea(tgeometry 'Point(1 1 1)@2000-01-01'));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(traversedArea(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(traversedArea(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'))) AS t(dp);
SELECT ST_AsText(traversedArea(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'));
SELECT ST_AsText(traversedArea(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT ST_AsText(traversedArea(tgeography 'Point(1.5 1.5 1.5)@2000-01-01'));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(traversedArea(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(traversedArea(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}')::geometry)) AS t(dp);
SELECT ST_AsText(traversedArea(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'));
SELECT ST_AsText(traversedArea(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'));

-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(traversedArea(tgeometry '{[Point(1 1)@2001-01-01], [Point(1 1)@2001-02-01], [Point(1 1)@2001-03-01]}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(traversedArea(tgeometry '{[Point(1 1)@2001-01-01], [Point(1 1)@2001-02-01], [Point(1 1)@2001-03-01]}'))) AS t(dp);
-- SELECT ST_AsText(traversedArea(tgeography '{[Point(1 1)@2001-01-01], [Point(1 1)@2001-02-01], [Point(1 1)@2001-03-01]}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(traversedArea(tgeography '{[Point(1 1)@2001-01-01], [Point(1 1)@2001-02-01], [Point(1 1)@2001-03-01]}')::geometry)) AS t(dp);

---------------------------------------------------------

SELECT asText(centroid(tgeometry '[Point(1 1)@2000-01-01, Linestring(1 1,3 3)@2000-01-02, Polygon((1 1,4 4,7 1,1 1))@2000-01-03]'));
SELECT asText(centroid(tgeography '[MultiPoint(1 1,4 4,7 1)@2000-01-01, Polygon((1 1,4 4,7 1,1 1))@2000-01-02]'),6);

---------------------------------------------------------
-- Only 2D is allowed for atGeometry/minusGeometry on tgeometry

SELECT asText(atGeometry(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeometry '[Point(0 3)@2000-01-01, Point(1 1)@2000-01-02, Point(3 2)@2000-01-03, Point(0 3)@2000-01-04]', geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))'));
SELECT astext(atGeometry(tgeometry '[Point(0 0)@2000-01-01, Point(3 3)@2000-01-02, Point(0 3)@2000-01-03, Point(
3 0)@2000-01-04]', 'Polygon((1 1,2 1,2 2,1 2,1 1))'));

SELECT asText(atGeometry(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));

SELECT asText(atGeometry(tgeometry '[Point(1 1)@2000-01-01]', geometry 'Linestring(0 0,1 1)'));
SELECT asText(atGeometry(tgeometry '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02]','Point(2 2)'));
SELECT asText(atGeometry(tgeometry '[Point(0 1)@2000-01-01,Point(5 1)@2000-01-05]', geometry 'Linestring(0 0,2 2,3 1,4 1,5 0)'));
SELECT asText(atGeometry(tgeometry '[Point(0 0)@2000-01-01]', geometry 'Polygon((0 1,1 2,2 1,1 0,0 1))'));
SELECT asText(atGeometry(tgeometry '{[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02],[Point(0 0)@2000-01-03]}', geometry 'Polygon((0 1,1 2,2 1,1 0,0 1))'));

-- NULL
SELECT asText(atGeometry(tgeometry '[Point(1 1)@2000-01-01]', geometry 'Linestring(2 2,3 3)'));
SELECT asText(atGeometry(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]}', geometry 'Linestring(0 1,1 2)'));
SELECT asText(atGeometry(tgeometry '[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02)', geometry 'Linestring(1 1,2 2)'));

/* Errors */
SELECT atGeometry(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)');
SELECT atGeometry(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT atGeometry(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');

SELECT asText(minusGeometry(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));

SELECT asText(minusGeometry(tgeometry '[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02)', geometry 'Linestring(0 1,2 1)'));
SELECT asText(minusGeometry(tgeometry '{[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02)}', geometry 'Linestring(0 1,2 1)'));
SELECT asText(minusGeometry(tgeometry '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02]','Point(2 2)'));
SELECT asText(minusGeometry(tgeometry '{[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02],[Point(3 3)@2000-01-03]}','Point(2 2)'));

/* Errors */
SELECT minusGeometry(tgeometry 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)');
SELECT minusGeometry(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT minusGeometry(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');

--------------------------------------------------------

-- Equivalences
WITH test(temp, geo) AS (
  SELECT tgeometry '[Point(1 1)@2000-01-01, Point(3 1)@2000-01-03,
    Point(3 1)@2000-01-05]', geometry 'Polygon((2 0,2 2,4 2,4 0,2 0))' )
SELECT temp = merge(atGeometry(temp, geo), minusGeometry(temp, geo))
FROM test;

--------------------------------------------------------

SELECT asText(atStbox(tgeometry 'Point(1 1)@2000-01-01', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));

SELECT asText(atStbox(tgeometry 'Point(1 1)@2000-01-01', 'STBOX X((1,1),(2,2))'));
SELECT asText(atStbox(tgeometry 'Point(1 1)@2000-01-01', 'STBOX T([2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tgeometry '(Point(2 2)@2000-01-02, Point(3 3)@2000-01-03]', 'STBOX T([2000-01-01,2000-01-02])'));

-- borderInc set to false
SELECT asText(atStbox(tgeometry '[Point(1 1)@2001-01-01, Point(1 0)@2001-01-02,
  Point(0 0)@2001-01-03, Point(0 1)@2001-01-04, Point(1 1)@2001-01-05]',
  stbox 'STBOX X((0,0),(2,2))', false));
SELECT asText(atStbox(tgeometry '[Point(1 1)@2001-01-01, Point(1 2)@2001-01-02,
  Point(2 2)@2001-01-03, Point(2 1)@2001-01-04, Point(1 1)@2001-01-05]',
  stbox 'STBOX X((0,0),(2,2))', false));

SELECT asText(minusStbox(tgeometry 'Point(1 1)@2000-01-01', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(minusStbox(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(minusStbox(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(minusStbox(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));

-- borderInc set to false
SELECT asText(minusStbox(tgeometry '[Point(1 1)@2001-01-01, Point(1 0)@2001-01-02,
  Point(0 0)@2001-01-03, Point(0 1)@2001-01-04, Point(1 1)@2001-01-05]',
  stbox 'STBOX X((0,0),(2,2))', false));
SELECT asText(minusStbox(tgeometry '[Point(1 1)@2001-01-01, Point(1 2)@2001-01-02,
  Point(2 2)@2001-01-03, Point(2 1)@2001-01-04, Point(1 1)@2001-01-05]',
  stbox 'STBOX X((0,0),(2,2))', false));

-- Instantaneous sequence
SELECT asText(atStbox(tgeometry '{Point(1 1)@2000-01-01}', stbox 'STBOX X((1 1),(3 3))'));
SELECT asText(atStbox(tgeometry '[Point(1 1)@2000-01-01]', stbox 'STBOX X((1 1),(3 3))'));

-- Edge cases
SELECT astext(atStbox(tgeometry '[Point(1 1)@2000-01-01, Point(1 3)@2000-01-02, Point(1 1)@2000-01-03]', stbox 'STBOX X((0,0),(2 2))'));
SELECT astext(atStbox(tgeometry '[Point(1 1)@2000-01-01, Point(1 2)@2000-01-02, Point(1 1)@2000-01-03]', stbox 'STBOX X((0,0),(2 2))'));
SELECT astext(atStbox(tgeometry '[Point(1 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 1)@2000-01-03]', stbox 'STBOX X((0,0),(1 1))'));
SELECT astext(atStbox(tgeometry '[Point(0 2)@2000-01-01, Point(2 0)@2000-01-02]', stbox 'STBOX X((0,0),(1 1))'));
SELECT astext(atStbox(tgeometry '[Point(0 0)@2000-01-01, Point(2 2)@2000-01-02]', stbox 'STBOX X((0,0),(1 1))'));
SELECT astext(atStbox(tgeometry '[Point(-2 1)@2000-01-01, Point(2 1)@2000-01-02]', stbox 'STBOX X((0,0),(1 1))'));

-- Edge cases with lower_inc / upper_inc = false
SELECT astext(atStbox(tgeometry '(Point(2 1)@2000-01-01, Point(2 1)@2000-01-02)', stbox 'STBOX X((0,0),(2 2))'));
SELECT astext(atStbox(tgeometry '(Point(3 1)@2000-01-01, Point(3 1)@2000-01-02)', stbox 'STBOX X((0,0),(2 2))'));
SELECT astext(atStbox(tgeometry '(Point(1 1)@2000-01-01, Point(3 1)@2000-01-02, Point(3 1)@2000-01-04)', stbox 'STBOX X((0,0),(2 2))'));
SELECT astext(atStbox(tgeometry '(Point(3 1)@2000-01-01, Point(2 1)@2000-01-02, Point(2 1)@2000-01-04)', stbox 'STBOX X((0,0),(2 2))'));
SELECT astext(atStbox(tgeometry '(Point(1 1)@2000-01-01, Point(3 1)@2000-01-02, Point(3 1)@2000-01-04)', stbox 'STBOX X((0,0),(2 2))'));
SELECT astext(atStbox(tgeometry '(Point(3 1)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-04)', stbox 'STBOX X((0,0),(2 2))'));

/* Errors */
SELECT asText(atStbox(tgeometry 'SRID=4326;Point(1 1)@2000-01-01', 'GEODSTBOX ZT(((1,1,1),(2,2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tgeometry 'SRID=5676;Point(1 1)@2000-01-01', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));

-------------------------------------------------------------------------------

-- Equivalences
WITH temp(trip, box) AS (
  SELECT tgeometry '[Point(1 1)@2000-01-01, Point(3 1)@2000-01-03,
    Point(3 1)@2000-01-05]', stbox 'STBox X((2 0),(4 2))' )
SELECT trip = merge(atStbox(trip, box), minusStbox(trip,box))
FROM temp;

--------------------------------------------------------

