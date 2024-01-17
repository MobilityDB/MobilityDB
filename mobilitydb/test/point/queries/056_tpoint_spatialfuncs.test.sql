-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2024, PostGIS contributors
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
-- Geoset

SELECT SRID(geomset '{"SRID=5676;Point(1 1)", "SRID=5676;Point(2 2)"}');
SELECT asEWKT(setSRID(geomset '{"Point(0 0)", "Point(1 1)"}', 5676));

-- Tests independent of the PROJ version
WITH test(geoset) AS (
  SELECT geomset 'SRID=4326;{"Point(1.0 2.0 3.0)","Point(4.0 5.0 6.0)"}' )
SELECT asEWKT(transform(transform(geoset, 5676), 4326), 6) FROM test;

-- Noop
SELECT asEWKT(transform(geomset 'SRID=4326;{"Point(1.0 2.0 3.0)","Point(4.0 5.0 6.0)"}', 4326));

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

-- Tests independent of the PROJ version
SELECT round(transform(transform(stbox 'SRID=4326;STBOX X((1,1),(2,2))', 5676), 4326), 1);
SELECT round(transform(transform(stbox 'SRID=4326;STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])', 5676), 4326), 1);
SELECT round(transform(transform(stbox 'SRID=4326;STBOX Z((1,1,1),(2,2,2))', 5676), 4326), 1);
SELECT round(transform(transform(stbox 'SRID=4326;STBOX ZT(((1,1,1),(2,2,2)),[2000-01-01,2000-01-02])', 5676), 4326), 1);
SELECT round(transform(transform(stbox 'SRID=4326;GEODSTBOX Z((1,1,1),(2,2,2))', 4269), 4326), 1);
SELECT round(transform(transform(stbox 'SRID=4326;GEODSTBOX ZT(((1,1,1),(2,2,2)),[2000-01-01,2000-01-02])', 4269), 4326), 1);

SELECT DISTINCT SRID(b) FROM tbl_stbox;
SELECT MIN(xmin(setSRID(b,4326))) FROM tbl_stbox;

SELECT ROUND(MIN(xmin(transform(transform(setSRID(b,5676), 4326), 5676)))::numeric, 1) FROM tbl_stbox;
SELECT MIN(xmin(round(transform(transform(setSRID(b,5676), 4326), 5676), 1))) FROM tbl_stbox;

-- Noop
SELECT transform(stbox 'SRID=4326;STBOX X((1,1),(2,2))', 4326);

/* Errors */
SELECT transform(stbox 'SRID=4326;STBOX X((1,1),(2,2))', 0);

-------------------------------------------------------------------------------
-- 2D
SELECT SRID(tgeompoint 'Point(1 1)@2000-01-01');
SELECT SRID(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT SRID(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT SRID(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT SRID(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT SRID(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT SRID(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT SRID(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
-- 3D
SELECT SRID(tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT SRID(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT SRID(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT SRID(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT SRID(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT SRID(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT SRID(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT SRID(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

SELECT asEWKT(setSRID(tgeompoint 'Point(1 1 1)@2000-01-01', 5676));
SELECT asEWKT(setSRID(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 5676));
SELECT asEWKT(setSRID(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 5676));
SELECT asEWKT(setSRID(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 5676));
SELECT asEWKT(setSRID(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', 4269));
SELECT asEWKT(setSRID(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', 4269));
SELECT asEWKT(setSRID(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', 4269));
SELECT asEWKT(setSRID(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 4269));

SELECT startValue(transform(tgeompoint 'SRID=5676;Point(1 2 3)@2000-01-01', 4326)) = st_transform(geometry 'SRID=5676;Point(1 2 3)', 4326);
SELECT startValue(transform(tgeompoint 'SRID=5676;{Point(1 2 3)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 2 3)@2000-01-03}', 4326)) = st_transform(geometry 'SRID=5676;Point(1 2 3)', 4326);
SELECT startValue(transform(tgeompoint 'SRID=5676;[Point(1 2 3)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 2 3)@2000-01-03]', 4326)) = st_transform(geometry 'SRID=5676;Point(1 2 3)', 4326);
SELECT startValue(transform(tgeompoint 'SRID=5676;{[Point(1 2 3)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 2 3)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 4326)) = st_transform(geometry 'SRID=5676;Point(1 2 3)', 4326);
-- Noop
SELECT startValue(transform(tgeompoint 'SRID=5676;Point(1 2 3)@2000-01-01', 5676)) = st_transform(geometry 'SRID=5676;Point(1 2 3)', 5676);
SELECT startValue(transform(tgeompoint 'SRID=5676;Point(1 2 3)@2000-01-01', 4326)) = st_transform(geometry 'SRID=4326;Point(1 2 3)', 4326);

-------------------------------------------------------------------------------
-- Transform with pipeline

WITH test(geoset, pipeline) AS (
  SELECT geomset 'SRID=4326;{"Point(1.0 2.0 3.0)","Point(4.0 5.0 6.0)"}',
   text 'urn:ogc:def:coordinateOperation:EPSG::16031' )
SELECT asText(transformPipeline(transformPipeline(geoset, pipeline, 4326), pipeline, 4326, false), 3) = asText(geoset, 3)
FROM test;

WITH test(box, pipeline) AS (
  SELECT tgeompoint 'SRID=4326;[POINT(2.123456 49.123456)@2000-01-01, POINT(3.123456 50.123456)@2000-01-02]'::stbox,
   text 'urn:ogc:def:coordinateOperation:EPSG::16031' )
SELECT asText(transformPipeline(transformPipeline(box, pipeline, 4326), pipeline, 4326, false), 3) = asText(box, 3)
FROM test;

-- This test ensure correctness between PostGIS and MobilityDB results.
-- However, it was commented out since not all installations have the latest 
-- PostGIS version 3.4
-- SELECT startValue(transformPipeline(tgeompoint 'SRID=4326;Point(1 2 3)@2000-01-01',
  -- 'urn:ogc:def:coordinateOperation:EPSG::16031', 5676, true)) = 
  -- st_transformpipeline(geometry 'SRID=4326;Point(1 2 3)', 'urn:ogc:def:coordinateOperation:EPSG::16031', 5676);

SELECT asText(transformPipeline(tgeompoint 'SRID=4326;POINT(2 49)@2000-01-01',
  text 'urn:ogc:def:coordinateOperation:EPSG::16031'));
SELECT asText(transformPipeline(tgeompoint 'POINT(426857.9877165967 5427937.523342293)@2000-01-01 00:00:00+01',
  text 'urn:ogc:def:coordinateOperation:EPSG::16031', 4326, false), 3);

WITH test1(pipeline) AS (
  SELECT text 'urn:ogc:def:coordinateOperation:EPSG::16031' ),
test2(temp) AS (
  SELECT tgeompoint 'SRID=4326;POINT(2.123456 49.123456)@2000-01-01' UNION
  SELECT tgeompoint 'SRID=4326;{POINT(2.123456 49.123456)@2000-01-01, POINT(3.123456 50.123456)@2000-01-02}' UNION
  SELECT tgeompoint 'SRID=4326;[POINT(2.123456 49.123456)@2000-01-01, POINT(3.123456 50.123456)@2000-01-02]' UNION
  SELECT tgeompoint 'SRID=4326;{[POINT(2.123456 49.123456)@2000-01-01, POINT(3.123456 50.123456)@2000-01-02],'
    '[POINT(2.123456 49.123456)@2000-01-03, POINT(3.123456 50.123456)@2000-01-04]}'
    )
SELECT DISTINCT asText(transformPipeline(transformPipeline(temp, pipeline, 4326), pipeline, 4326, false), 3) = asText(temp, 3)
FROM test1, test2;

--------------------------------------------------------

-- Temporal type
SELECT asEWKT(round(transform_gk(tgeompoint 'Point(13.43593 52.41721)@2018-12-20'), 6));
SELECT asEWKT(round(transform_gk(tgeompoint '{Point(13.43593 52.41721)@2018-12-20 10:00:00, Point(13.43605 52.41723)@2018-12-20 10:01:00}'), 6));
SELECT asEWKT(round(transform_gk(tgeompoint '[Point(13.43593 52.41721)@2018-12-20 10:00:00, Point(13.43605 52.41723)@2018-12-20 10:01:00]'), 6));
SELECT asEWKT(round(transform_gk(tgeompoint '{[Point(13.43593 52.41721)@2018-12-20 10:00:00, Point(13.43605 52.41723)@2018-12-20 10:01:00],[Point(13.43705 52.41724)@2018-12-20 10:02:00,Point(13.43805 52.41730)@2018-12-20 10:03:00]}'), 6));

-- PostGIS geometry
SELECT ST_AsText(round(transform_gk(geometry 'Point Empty'), 6));
SELECT ST_AsText(round(transform_gk(geometry 'Point(13.43593 52.41721)'), 6));
SELECT ST_AsText(round(geometry 'Linestring empty', 6));
SELECT ST_AsText(round(transform_gk(geometry 'Linestring(13.43593 52.41721,13.43593 52.41723)'), 6));

/* Error */
SELECT transform_gk(round(geometry 'Polygon((0 0,0 10,10 10,10 0,0 0))', 6));

--------------------------------------------------------

-- 2D
SELECT ST_AsText(round(geometry 'Point(1.123456789 1.123456789)', 6));
SELECT ST_AsText(round(geometry 'Linestring(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789)', 6));
SELECT ST_AsText(round(geometry 'Triangle((1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 1.123456789,1.123456789 1.123456789))', 6));
SELECT ST_AsText(round(geometry 'Circularstring(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789)', 6));
SELECT ST_AsText(round(geometry 'Polygon((1.123456789 1.123456789,4.123456789 4.123456789,7.123456789 1.123456789,1.123456789 1.123456789),(3.123456789 2.123456789,4.123456789 3.123456789,5.123456789 2.123456789,3.123456789 2.123456789))', 6));
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

SELECT ST_AsText(round(geometry 'Point Empty', 6));
SELECT ST_AsText(round(geometry 'Linestring Empty', 6));
SELECT ST_AsText(round(geometry 'Triangle Empty', 6));
SELECT ST_AsText(round(geometry 'Circularstring Empty', 6));
SELECT ST_AsText(round(geometry 'Polygon Empty', 6));
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
SELECT asText(round(tgeompoint 'Point(1.12345 1.12345)@2000-01-01', 2));
SELECT asText(round(tgeompoint '{Point(1.12345 1.12345)@2000-01-01, Point(2 2)@2000-01-02, Point(1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(round(tgeompoint '[Point(1.12345 1.12345)@2000-01-01, Point(2 2)@2000-01-02, Point(1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(round(tgeompoint '{[Point(1.12345 1.12345)@2000-01-01, Point(2 2)@2000-01-02, Point(1.12345 1.12345)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2));
SELECT asText(round(tgeogpoint 'Point(1.12345 1.12345)@2000-01-01', 2));
SELECT asText(round(tgeogpoint '{Point(1.12345 1.12345)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(round(tgeogpoint '[Point(1.12345 1.12345)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(round(tgeogpoint '{[Point(1.12345 1.12345)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.12345 1.12345)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2));
-- 3D
SELECT asText(round(tgeompoint 'Point(1.12345 1.12345 1.12345)@2000-01-01', 2));
SELECT asText(round(tgeompoint '{Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(round(tgeompoint '[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(round(tgeompoint '{[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2));
SELECT asText(round(tgeogpoint 'Point(1.12345 1.12345 1.12345)@2000-01-01', 2));
SELECT asText(round(tgeogpoint '{Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(round(tgeogpoint '[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(round(tgeogpoint '{[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2));

SELECT asText(round(ARRAY[tgeompoint '[Point(1.55 1.55)@2000-01-01, Point(2.55 2.55)@2000-01-02, Point(1.55 1.55)@2000-01-03]', '[Point(3.55 3.55)@2000-01-04, Point(3.55 3.55)@2000-01-05]'],1));
SELECT round(ARRAY[]::tgeompoint[]);

--------------------------------------------------------

SELECT asEWKT(tgeompoint 'Point(1 1)@2000-01-01'::tgeogpoint);
SELECT asEWKT(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'::tgeogpoint);
SELECT asEWKT(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'::tgeogpoint);
SELECT asEWKT(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'::tgeogpoint);
SELECT asEWKT(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'::tgeogpoint);
SELECT asEWKT(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'::tgeogpoint);

SELECT asEWKT(tgeogpoint 'Point(1.5 1.5)@2000-01-01'::tgeompoint);
SELECT asEWKT(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'::tgeompoint);
SELECT asEWKT(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'::tgeompoint);
SELECT asEWKT(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'::tgeompoint);
SELECT asEWKT(tgeogpoint 'Interp=Step;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'::tgeompoint);
SELECT asEWKT(tgeogpoint 'Interp=Step;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'::tgeompoint);

--------------------------------------------------------
--2D
SELECT getX(tgeompoint 'Point(1 1)@2000-01-01');
SELECT getX(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT getX(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT getX(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT getX(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT getX(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT getX(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT getX(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT getX(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT getX(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
SELECT getX(tgeogpoint 'Interp=Step;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT getX(tgeogpoint 'Interp=Step;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT getY(tgeompoint 'Point(1 1)@2000-01-01');
SELECT getY(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT getY(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT getY(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT getY(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT getY(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT getY(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT getY(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT getY(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT getY(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
SELECT getY(tgeogpoint 'Interp=Step;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT getY(tgeogpoint 'Interp=Step;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

/* Errors */
SELECT getZ(tgeompoint 'Point(1 1)@2000-01-01');
SELECT getZ(tgeogpoint 'Point(1.5 1.5)@2000-01-01');

-- 3D
SELECT getX(tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT getX(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT getX(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT getX(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT getX(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT getX(tgeompoint 'Interp=Step;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT getX(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT getX(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT getX(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT getX(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');
SELECT getX(tgeogpoint 'Interp=Step;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT getX(tgeogpoint 'Interp=Step;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

SELECT getY(tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT getY(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT getY(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT getY(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT getY(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT getY(tgeompoint 'Interp=Step;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT getY(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT getY(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT getY(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT getY(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');
SELECT getY(tgeogpoint 'Interp=Step;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT getY(tgeogpoint 'Interp=Step;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

SELECT getZ(tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT getZ(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT getZ(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT getZ(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT getZ(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT getZ(tgeompoint 'Interp=Step;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT getZ(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT getZ(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT getZ(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT getZ(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');
SELECT getZ(tgeogpoint 'Interp=Step;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT getZ(tgeogpoint 'Interp=Step;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

--------------------------------------------------------

-- 2D
SELECT ST_AsText(trajectory(tgeompoint 'Point(1 1)@2000-01-01'));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(trajectory(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(trajectory(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'))) AS t(dp);
SELECT ST_AsText(trajectory(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(trajectory(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(trajectory(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'))) AS t(dp);
-- SELECT ST_AsText(trajectory(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(trajectory(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'))) AS t(dp);
SELECT ST_AsText(trajectory(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(trajectory(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(trajectory(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}')::geometry)) AS t(dp);
SELECT ST_AsText(trajectory(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(trajectory(tgeogpoint 'Interp=Step;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(trajectory(tgeogpoint 'Interp=Step;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')::geometry)) AS t(dp);
-- SELECT ST_AsText(trajectory(tgeogpoint 'Interp=Step;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(trajectory(tgeogpoint 'Interp=Step;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::geometry)) AS t(dp);
-- 3D
SELECT ST_AsText(trajectory(tgeompoint 'Point(1 1 1)@2000-01-01'));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(trajectory(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(trajectory(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'))) AS t(dp);
SELECT ST_AsText(trajectory(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT ST_AsText(trajectory(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01'));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(trajectory(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(trajectory(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}')::geometry)) AS t(dp);
SELECT ST_AsText(trajectory(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'));

-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(trajectory(tgeompoint '{[Point(1 1)@2001-01-01], [Point(1 1)@2001-02-01], [Point(1 1)@2001-03-01]}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(trajectory(tgeompoint '{[Point(1 1)@2001-01-01], [Point(1 1)@2001-02-01], [Point(1 1)@2001-03-01]}'))) AS t(dp);
-- SELECT ST_AsText(trajectory(tgeogpoint '{[Point(1 1)@2001-01-01], [Point(1 1)@2001-02-01], [Point(1 1)@2001-03-01]}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(trajectory(tgeogpoint '{[Point(1 1)@2001-01-01], [Point(1 1)@2001-02-01], [Point(1 1)@2001-03-01]}')::geometry)) AS t(dp);
-- SELECT ST_AsText(trajectory(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02],[Point(1 1)@2000-01-03, Point(2 2)@2000-01-04]}'));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(trajectory(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02],[Point(1 1)@2000-01-03, Point(2 2)@2000-01-04]}'))) AS t(dp);

--------------------------------------------------------

-- 2D
SELECT round(length(tgeompoint 'Point(1 1)@2000-01-01')::numeric, 6);
SELECT round(length(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}')::numeric, 6);
SELECT round(length(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeogpoint 'Point(1.5 1.5)@2000-01-01')::numeric, 6);
SELECT round(length(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}')::numeric, 6);
SELECT round(length(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeogpoint 'Interp=Step;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeogpoint 'Interp=Step;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);
-- 3D
SELECT round(length(tgeompoint 'Point(1 1 1)@2000-01-01')::numeric, 6);
SELECT round(length(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}')::numeric, 6);
SELECT round(length(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeompoint 'Interp=Step;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01')::numeric, 6);
SELECT round(length(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}')::numeric, 6);
SELECT round(length(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeogpoint 'Interp=Step;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeogpoint 'Interp=Step;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);

-- 2D
SELECT round(cumulativeLength(tgeompoint 'Point(1 1)@2000-01-01'), 6);
SELECT round(cumulativeLength(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'), 6);
SELECT round(cumulativeLength(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Point(1.5 1.5)@2000-01-01'), 6);
SELECT round(cumulativeLength(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'), 6);
SELECT round(cumulativeLength(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Interp=Step;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Interp=Step;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'), 6);
-- 3D
SELECT round(cumulativeLength(tgeompoint 'Point(1 1 1)@2000-01-01'), 6);
SELECT round(cumulativeLength(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'), 6);
SELECT round(cumulativeLength(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeompoint 'Interp=Step;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01'), 6);
SELECT round(cumulativeLength(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'), 6);
SELECT round(cumulativeLength(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Interp=Step;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Interp=Step;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'), 6);

-- 2D
SELECT round(speed(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round(speed(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round(speed(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'), 6);
SELECT round(speed(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'), 6);
-- 3D
SELECT round(speed(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round(speed(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(speed(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'), 6);
SELECT round(speed(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'), 6);
/* Errors */
SELECT round(speed(tgeompoint 'Point(1 1)@2000-01-01'), 6);
SELECT round(speed(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'), 6);
SELECT round(speed(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round(speed(tgeompoint 'Interp=Step;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(speed(tgeogpoint 'Point(1.5 1.5)@2000-01-01'), 6);
SELECT round(speed(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'), 6);
SELECT round(speed(tgeogpoint 'Interp=Step;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'), 6);
SELECT round(speed(tgeogpoint 'Interp=Step;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'), 6);

SELECT direction(tgeompoint '[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT direction(tgeompoint '{[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02]}');

-- 2D
SELECT ST_AsText(round(twcentroid(tgeompoint 'Point(1 1)@2000-01-01'), 6));
SELECT ST_AsText(round(twcentroid(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'), 6));
SELECT ST_AsText(round(twcentroid(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6));
SELECT ST_AsText(round(twcentroid(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6));
SELECT ST_AsText(round(twcentroid(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6));
SELECT ST_AsText(round(twcentroid(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6));
-- 3D
SELECT ST_AsText(round(twcentroid(tgeompoint 'Point(1 1 1)@2000-01-01'), 6));
SELECT ST_AsText(round(twcentroid(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'), 6));
SELECT ST_AsText(round(twcentroid(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6));
SELECT ST_AsText(round(twcentroid(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6));
SELECT ST_AsText(round(twcentroid(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6));
SELECT ST_AsText(round(twcentroid(tgeompoint 'Interp=Step;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6));

-- 2D
SELECT round(degrees(azimuth(tgeompoint 'Point(1 1)@2000-01-01')), 6);
SELECT round(degrees(azimuth(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}')), 6);
SELECT round(degrees(azimuth(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')), 6);
SELECT round(degrees(azimuth(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')), 6);
-- Return negative result in PostGIS 2.5.5
-- SELECT round(degrees(azimuth(tgeogpoint 'Point(1.5 1.5)@2000-01-01')), 6);
-- SELECT round(degrees(azimuth(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}')), 6);
-- SELECT round(degrees(azimuth(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')), 6);
-- SELECT round(degrees(azimuth(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);
-- SELECT round(degrees(azimuth(tgeogpoint 'Interp=Step;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')), 6);
-- SELECT round(degrees(azimuth(tgeogpoint 'Interp=Step;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);
-- 3D
SELECT round(degrees(azimuth(tgeompoint 'Point(1 1 1)@2000-01-01')), 6);
SELECT round(degrees(azimuth(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}')), 6);
SELECT round(degrees(azimuth(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')), 6);
SELECT round(degrees(azimuth(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint 'Interp=Step;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')), 6);

SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02]')), 6);

SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]')), 6);
SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02)')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(1 1)@2000-01-02)')), 6);

SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02, Point(1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02, Point(1 1)@2000-01-03)')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(0 0)@2000-01-02, Point(1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(0 0)@2000-01-02, Point(1 1)@2000-01-03)')), 6);

SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03)')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03)')), 6);

SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03, Point(0 0)@2000-01-04]')), 6);
SELECT round(degrees(azimuth(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03, Point(0 0)@2000-01-04)')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03, Point(0 0)@2000-01-04]')), 6);
SELECT round(degrees(azimuth(tgeompoint '(Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03, Point(0 0)@2000-01-04)')), 6);

-- Return negative result in PostGIS 2.5.5, return erroneous value in PostGIS 3.1.1
-- 2D
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint 'Point(1.5 1.5)@2000-01-01')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint 'Interp=Step;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint 'Interp=Step;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6) IS NOT NULL;
-- 3D
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint 'Interp=Step;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint 'Interp=Step;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6) IS NOT NULL;

SELECT 1 WHERE round(degrees(azimuth(tgeogpoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]}')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint '{[Point(1 1)@2000-01-01], [Point(2 2)@2000-01-02], [Point(1 1)@2000-01-03]}')), 6) IS NOT NULL;

--------------------------------------------------------

SELECT degrees(bearing(geometry 'Point(0 0)', geometry 'Point(0 0)'));
SELECT degrees(bearing(geometry 'Point(0 0)', geometry 'Point(1 0)'));
SELECT degrees(bearing(geometry 'Point(0 0)', geometry 'Point(1 1)'));
SELECT degrees(bearing(geometry 'Point(0 0)', geometry 'Point(0 1)'));
SELECT degrees(bearing(geometry 'Point(0 0)', geometry 'Point(-1 1)'));
SELECT degrees(bearing(geometry 'Point(0 0)', geometry 'Point(-1 0)'));
SELECT degrees(bearing(geometry 'Point(0 0)', geometry 'Point(-1 -1)'));
SELECT degrees(bearing(geometry 'Point(0 0)', geometry 'Point(0 -1)'));
SELECT degrees(bearing(geometry 'Point(0 0)', geometry 'Point(1 -1)'));

SELECT degrees(bearing(geometry 'Point(0 0)', geometry 'Point empty'));

SELECT round(degrees(bearing(geography 'Point(0 0)', geography 'Point(0 0)'))::numeric, 6);
SELECT round(degrees(bearing(geography 'Point(0 0)', geography 'Point(1 0)'))::numeric, 6);
SELECT round(degrees(bearing(geography 'Point(0 0)', geography 'Point(1 1)'))::numeric, 6);
SELECT round(degrees(bearing(geography 'Point(0 0)', geography 'Point(0 1)'))::numeric, 6);
SELECT round(degrees(bearing(geography 'Point(0 0)', geography 'Point(-1 1)'))::numeric, 6);
SELECT round(degrees(bearing(geography 'Point(0 0)', geography 'Point(-1 0)'))::numeric, 6);
SELECT round(degrees(bearing(geography 'Point(0 0)', geography 'Point(-1 -1)'))::numeric, 6);
SELECT round(degrees(bearing(geography 'Point(0 0)', geography 'Point(0 -1)'))::numeric, 6);
SELECT round(degrees(bearing(geography 'Point(0 0)', geography 'Point(1 -1)'))::numeric, 6);

SELECT round(degrees(bearing(geometry 'Point(1 1)', tgeompoint 'Point(2 2)@2000-01-01')), 6);
SELECT round(degrees(bearing(geometry 'Point(1 1)', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')), 6);
SELECT round(degrees(bearing(geometry 'Point(1 1)', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')), 6);
SELECT round(degrees(bearing(geometry 'Point(2 2)', tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]')), 6);
SELECT round(degrees(bearing(geometry 'Point(1 1)', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')), 6);

SELECT round(degrees(bearing(geometry 'Point empty', tgeompoint 'Point(2 2)@2000-01-01')), 6);
SELECT round(degrees(bearing(geometry 'Point empty', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')), 6);
SELECT round(degrees(bearing(geometry 'Point empty', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')), 6);
SELECT round(degrees(bearing(geometry 'Point empty', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')), 6);

SELECT round(degrees(bearing(geometry 'Point(1 1 1)', tgeompoint 'Point(2 2 2)@2000-01-01')), 6);
SELECT round(degrees(bearing(geometry 'Point(1 1 1)', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')), 6);
SELECT round(degrees(bearing(geometry 'Point(1 1 1)', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')), 6);
SELECT round(degrees(bearing(geometry 'Point(1 1 1)', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')), 6);

SELECT round(degrees(bearing(geometry 'Point Z empty', tgeompoint 'Point(2 2 2)@2000-01-01')), 6);
SELECT round(degrees(bearing(geometry 'Point Z empty', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')), 6);
SELECT round(degrees(bearing(geometry 'Point Z empty', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')), 6);
SELECT round(degrees(bearing(geometry 'Point Z empty', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')), 6);

SELECT round(degrees(bearing(geography 'Point(1 1)', tgeogpoint 'Point(2.5 2.5)@2000-01-01')), 6);
SELECT round(degrees(bearing(geography 'Point(1 1)', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')), 6);
SELECT round(degrees(bearing(geography 'Point(1 1)', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')), 6);
SELECT round(degrees(bearing(geography 'Point(1 1)', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);

SELECT round(degrees(bearing(geography 'Point empty', tgeogpoint 'Point(2.5 2.5)@2000-01-01')), 6);
SELECT round(degrees(bearing(geography 'Point empty', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')), 6);
SELECT round(degrees(bearing(geography 'Point empty', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')), 6);
SELECT round(degrees(bearing(geography 'Point empty', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);

SELECT round(degrees(bearing(geography 'Point(1 1 1)', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')), 6);
SELECT round(degrees(bearing(geography 'Point(1 1 1)', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')), 6);
SELECT round(degrees(bearing(geography 'Point(1 1 1)', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')), 6);
SELECT round(degrees(bearing(geography 'Point(1 1 1)', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6);

SELECT round(degrees(bearing(geography 'Point Z empty', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')), 6);
SELECT round(degrees(bearing(geography 'Point Z empty', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')), 6);
SELECT round(degrees(bearing(geography 'Point Z empty', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')), 6);
SELECT round(degrees(bearing(geography 'Point Z empty', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6);

SELECT round(degrees(bearing(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1)')), 6);
SELECT round(degrees(bearing(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)')), 6);
SELECT round(degrees(bearing(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)')), 6);
SELECT round(degrees(bearing(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)')), 6);

SELECT round(degrees(bearing(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point empty')), 6);
SELECT round(degrees(bearing(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point empty')), 6);
SELECT round(degrees(bearing(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point empty')), 6);
SELECT round(degrees(bearing(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point empty')), 6);

SELECT round(degrees(bearing(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1 1)')), 6);
SELECT round(degrees(bearing(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Point(1 1 1)')), 6);
SELECT round(degrees(bearing(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Point(1 1 1)')), 6);
SELECT round(degrees(bearing(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Point(1 1 1)')), 6);

SELECT round(degrees(bearing(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Point Z empty')), 6);
SELECT round(degrees(bearing(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Point Z empty')), 6);
SELECT round(degrees(bearing(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Point Z empty')), 6);
SELECT round(degrees(bearing(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Point Z empty')), 6);

-- Test for adding a turning point if the point to the North of the line
SELECT round(degrees(bearing(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', geometry 'Point(2 3)')), 6);
SELECT round(degrees(bearing(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', geometry 'Point(2 2)')), 6);
SELECT round(degrees(bearing(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-03]', geometry 'Point(2 1)')), 6);

SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5)@2000-01-01', geography 'Point(1 1)')), 6);
SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', geography 'Point(1 1)')), 6);
SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', geography 'Point(1 1)')), 6);
SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', geography 'Point(1 1)')), 6);

SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5)@2000-01-01', geography 'Point empty')), 6);
SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', geography 'Point empty')), 6);
SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', geography 'Point empty')), 6);
SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', geography 'Point empty')), 6);

SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', geography 'Point(1 1 1)')), 6);
SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', geography 'Point(1 1 1)')), 6);
SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', geography 'Point(1 1 1)')), 6);
SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', geography 'Point(1 1 1)')), 6);

SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', geography 'Point Z empty')), 6);
SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', geography 'Point Z empty')), 6);
SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', geography 'Point Z empty')), 6);
SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', geography 'Point Z empty')), 6);

SELECT round(degrees(bearing(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(2 2)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(2 2)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(2 2)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(2 2)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')), 6);
SELECT round(degrees(bearing(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')), 6);
SELECT round(degrees(bearing(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')), 6);
SELECT round(degrees(bearing(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')), 6);

SELECT round(degrees(bearing(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint 'Point(2 2 2)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint 'Point(2 2 2)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint 'Point(2 2 2)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint 'Point(2 2 2)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')), 6);
SELECT round(degrees(bearing(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')), 6);
SELECT round(degrees(bearing(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')), 6);
SELECT round(degrees(bearing(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{[Point(2 2 2)@2000-01-01, Point(1 1 1)@2000-01-02, Point(2 2 2)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')), 6);

SELECT round(degrees(bearing(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint 'Interp=Step;[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02]')), 6);

-- SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint 'Point(2.5 2.5)@2000-01-01')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint 'Point(2.5 2.5)@2000-01-01')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint 'Point(2.5 2.5)@2000-01-01')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(2.5 2.5)@2000-01-01')), 6);
-- SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')), 6);
-- SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')), 6);
-- SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);

-- SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')), 6);
-- SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')), 6);
-- SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')), 6);
-- SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6);
-- SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6);

/* Errors */
SELECT bearing(geometry 'Point(1 1)', geometry 'PointZ(2 2 2)');

--------------------------------------------------------

SELECT isSimple(tgeompoint '{Point(0 0)@2000-01-01}');
SELECT isSimple(tgeompoint '{Point(0 0)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT isSimple(tgeompoint '{Point(0 0)@2000-01-01, Point(0 0)@2000-01-02}');
SELECT isSimple(tgeompoint 'Interp=Step;[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT isSimple(tgeompoint 'Interp=Step;[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(2 0)@2000-01-03]');
SELECT isSimple(tgeompoint '[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT isSimple(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT isSimple(tgeompoint '[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02, Point(1 1)@2000-01-03, Point(1 1)@2000-01-04]');

SELECT isSimple(tgeompoint '{Point(0 0 0)@2000-01-01}');
SELECT isSimple(tgeompoint '{Point(0 0 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT isSimple(tgeompoint '{Point(0 0 0)@2000-01-01, Point(0 0 0)@2000-01-02}');

SELECT astext(unnest(makeSimple(tgeompoint '{Point(0 0)@2000-01-01}')));
SELECT astext(unnest(makeSimple(tgeompoint '{Point(0 0)@2000-01-01, Point(1 1)@2000-01-02}')));
SELECT astext(unnest(makeSimple(tgeompoint '{Point(0 0)@2000-01-01, Point(0 0)@2000-01-02}')));
SELECT astext(unnest(makeSimple(tgeompoint '{Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(0 0)@2000-01-03}')));

SELECT astext(unnest(makeSimple(tgeompoint '{Point(0 0 0)@2000-01-01}')));
SELECT astext(unnest(makeSimple(tgeompoint '{Point(0 0 0)@2000-01-01, Point(1 1 1)@2000-01-02}')));
SELECT astext(unnest(makeSimple(tgeompoint '{Point(0 0 0)@2000-01-01, Point(0 0 0)@2000-01-02}')));

-- Collinear tests
SELECT asText(makeSimple(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02, Point(2 2)@2000-01-03]'));
SELECT asText(makeSimple(tgeompoint '[Point(1 1)@2000-01-01, Point(1 3)@2000-01-02, Point(1 2)@2000-01-03]'));
SELECT asText(makeSimple(tgeompoint '[Point(1 1)@2000-01-01, Point(3 1)@2000-01-02, Point(2 1)@2000-01-03]'));
SELECT asText(unnest(makeSimple(tgeompoint '[Point(1 1)@2000-01-01, Point(2 1)@2000-01-02, Point(3 2)@2000-01-03, Point(3 1)@2000-01-04, Point(2 1)@2000-01-05]')));
SELECT asText(unnest(makeSimple(tgeompoint '[Point(1 1)@2000-01-01, Point(2 1)@2000-01-02, Point(2 2)@2000-01-03, Point(0 1)@2000-01-04, Point(1 1)@2000-01-05]')));

-- Exhaustive tests
SELECT asText(makeSimple(tgeompoint
  '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-02}'));
SELECT asText(makeSimple(tgeompoint
  '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asText(makeSimple(tgeompoint
  '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asText(makeSimple(tgeompoint
  '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03, Point(2 2)@2000-01-04}'));

SELECT asText(makeSimple(tgeompoint
  'Interp=Step;[Point(1 1)@2000-01-01, Point(1 1)@2000-01-03]'));
SELECT asText(makeSimple(tgeompoint
  'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asText(makeSimple(tgeompoint
  'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03, Point(2 2)@2000-01-04]'));

SELECT asText(makeSimple(tgeompoint
  '[Point(1 1)@2000-01-01, Point(1 1)@2000-01-03]'));
SELECT asText(makeSimple(tgeompoint
  '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asText(makeSimple(tgeompoint
  '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03, Point(2 2)@2000-01-04]'));

--------------------------------------------------------

-- Test for NULL inputs since the function is not STRICT
SELECT asText(atGeometry(NULL::tgeompoint, geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint 'Point(1 1)@2000-01-01', NULL::geometry));

-- 2D
SELECT asText(atGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint '[Point(0 3)@2000-01-01, Point(1 1)@2000-01-02, Point(3 2)@2000-01-03, Point(0 3)@2000-01-04]', geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))'));
SELECT astext(atGeometry(tgeompoint '[Point(0 0)@2000-01-01, Point(3 3)@2000-01-02, Point(0 3)@2000-01-03, Point(
3 0)@2000-01-04]', 'Polygon((1 1,2 1,2 2,1 2,1 1))'));
SELECT asText(atGeometry(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint 'Interp=Step;[Point(0 3)@2000-01-01, Point(1 1)@2000-01-02, Point(3 2)@2000-01-03, Point(0 3)@2000-01-04]', geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))'));

SELECT asText(atGeometry(tgeompoint 'Interp=Step;(Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(2 2)@2000-01-03)', geometry 'Linestring(1 1,2 2)'));
SELECT asText(atGeometry(tgeompoint 'Interp=Step;(Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(2 2)@2000-01-03)', geometry 'Point(2 2)'));
SELECT asText(atGeometry(tgeompoint 'Interp=Step;[Point(4 4)@2000-01-01, Point(3 3)@2000-01-02]', geometry 'Polygon((0 0,0 3,3 3,3 0,0 0))'));
SELECT asText(atGeometry(tgeompoint 'Interp=Step;[Point(4 4)@2000-01-01, Point(5 5)@2000-01-02, Point(3 3)@2000-01-03]', geometry 'Polygon((00 0,0 3,3 3,3 0,0 0))'));

SELECT asText(atGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));

-- 3D
SELECT asText(atGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint 'Interp=Step;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty'));
SELECT asText(atGeometry(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint 'Interp=Step;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring empty'));

SELECT asText(atGeometry(tgeompoint '[Point(1 1 1)@2000-01-01, Point(1 1 1)@2000-01-02]', geometry 'Linestring(0 0,0 2,2 2)'));

-- Period
SELECT asText(atGeometryTime(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01]', geometry 'Linestring(0 0, 2 2)', NULL, tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(atGeometryTime(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02, Point(2 2)@2000-01-03]', geometry 'Linestring(0 0, 2 2)', NULL, tstzspan '[2000-01-01, 2000-01-02]'));

--3D Zspan
SELECT asText(atGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0, 2 2)', floatspan '[0, 2]'));
select asText(atGeometry(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(1 1 3)@2000-01-02]', geometry 'Point(1 1)', floatspan '[1, 2]'));

SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01]', geometry 'Linestring(0 0,1 1)'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02]','Point(2 2)'));
SELECT asText(atGeometry(tgeompoint '[Point(0 1)@2000-01-01,Point(5 1)@2000-01-05]', geometry 'Linestring(0 0,2 2,3 1,4 1,5 0)'));
SELECT atGeometry(tgeompoint '[Point(0 0)@2000-01-01]', geometry 'Polygon((0 1,1 2,2 1,1 0,0 1))');
SELECT atGeometry(tgeompoint '{[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02],[Point(0 0)@2000-01-03]}', geometry 'Polygon((0 1,1 2,2 1,1 0,0 1))');

-- NULL
SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01]', geometry 'Linestring(2 2,3 3)'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]}', geometry 'Linestring(0 1,1 2)'));
SELECT asText(atGeometry(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02)', geometry 'Linestring(1 1,2 2)'));

/* Errors */
SELECT atGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT atGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');

-- 2D
SELECT asText(minusGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));
-- 3D
SELECT asText(minusGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Step;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Step;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Step;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring empty'));

SELECT asText(minusGeometry(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02)', geometry 'Linestring(0 1,2 1)'));
SELECT asText(minusGeometry(tgeompoint '{[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02)}', geometry 'Linestring(0 1,2 1)'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02]','Point(2 2)'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02],[Point(3 3)@2000-01-03]}','Point(2 2)'));

/* Errors */
SELECT minusGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');
SELECT minusGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)');

--------------------------------------------------------

-- Equivalences
WITH temp(trip, geo, zspan) AS (
  SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(3 1 1)@2000-01-03,
    Point(3 1 3)@2000-01-05]', geometry 'Polygon((2 0,2 2,4 2,4 0,2 0))', floatspan '[0,2]' )
SELECT trip = merge(atGeometry(trip, geo, zspan), minusGeometry(trip, geo, zspan))
FROM temp;

WITH temp(trip, geo, period) AS (
  SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(5 1)@2000-01-05, Point(1 1)@2000-01-09]',
    geometry 'Polygon((2 0,2 2,4 2,4 0,2 0))', tstzspan '[2000-01-03, 2000-01-05]' )
SELECT trip = merge(atGeometryTime(trip, geo, NULL::floatspan, period),
  minusGeometryTime(trip, geo, NULL::floatspan, period)) FROM temp;

WITH temp(trip, geo, zspan, period) AS (
  SELECT tgeompoint '[Point(1 1 1)@2000-01-01,
    Point(5 1 5)@2000-01-05, Point(1 1 9)@2000-01-09]',
    geometry 'Polygon((2 0,2 2,4 2,4 0,2 0))', floatspan '[0,5]',
    tstzspan '[2000-01-03, 2000-01-05]' )
SELECT trip = merge(atGeometryTime(trip, geo, zspan, period),
  minusGeometryTime(trip, geo, zspan, period)) FROM temp;

--------------------------------------------------------

SELECT asText(atStbox(tgeompoint 'Point(1 1)@2000-01-01', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));

SELECT asText(atStbox(tgeompoint 'Point(1 1)@2000-01-01', 'STBOX X((1,1),(2,2))'));
SELECT asText(atStbox(tgeompoint 'Point(1 1)@2000-01-01', 'STBOX T([2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tgeompoint '(Point(2 2)@2000-01-02, Point(3 3)@2000-01-03]', 'STBOX T([2000-01-01,2000-01-02])'));

SELECT asText(minusStbox(tgeompoint 'Point(1 1)@2000-01-01', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(minusStbox(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(minusStbox(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(minusStbox(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(minusStbox(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(minusStbox(tgeompoint 'Interp=Step;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));

-- Instantaneous sequence
SELECT asText(atStbox(tgeompoint '{Point(1 1)@2000-01-01}', stbox 'STBOX X((1 1),(3 3))'));
SELECT asText(atStbox(tgeompoint 'Interp=Step;[Point(1 1)@2000-01-01]', stbox 'STBOX X((1 1),(3 3))'));
SELECT asText(atStbox(tgeompoint '[Point(1 1)@2000-01-01]', stbox 'STBOX X((1 1),(3 3))'));

-- 3D
SELECT asText(atStbox(tgeompoint '[Point(2 0 2)@2000-01-01, Point(2 4 2)@2000-01-02]', stbox 'STBOX Z((1 1 1),(3
3 3))'));
SELECT atStbox (tgeompoint '{Point(1 2 1)@2000-01-01, Point(1 1 3)@2000-01-02}', stbox 'STBOX Z((0 0 0),(1 1 1))');

-- Mix 2D/3D
SELECT asText(atStbox(tgeompoint 'Point(1 1)@2000-01-01', 'STBOX ZT(((1,1,1),(2,2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tgeompoint '[Point(1 3 2)@2000-01-01, Point(3 1 2)@2000-01-03]', stbox 'STBOX X((0 0),(2 2)
)'));

/* Errors */
SELECT asText(atStbox(tgeompoint 'SRID=4326;Point(1 1)@2000-01-01', 'GEODSTBOX ZT(((1,1,1),(2,2,2)),[2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tgeompoint 'SRID=5676;Point(1 1)@2000-01-01', 'STBOX XT(((1,1),(2,2)),[2000-01-01,2000-01-02])'));

-------------------------------------------------------------------------------

-- Equivalences
WITH temp(trip, box) AS (
  SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(3 1 1)@2000-01-03,
    Point(3 1 3)@2000-01-05]', stbox 'STBox Z((2 0 0),(4 2 2))' )
SELECT trip = merge(atStbox(trip, box), minusStbox(trip,box))
FROM temp;

--------------------------------------------------------

