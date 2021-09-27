-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
--
-- Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2021, PostGIS contributors
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
-- STBOX

SELECT srid(stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))');
SELECT srid(stbox 'SRID=4326;STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))');
SELECT srid(stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))');

SELECT setSRID(stbox 'STBOX((1,1),(2,2))', 5676);
SELECT setSRID(stbox 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))', 5676);
SELECT setSRID(stbox 'STBOX Z((1,1,1),(2,2,2))', 5676);
SELECT setSRID(stbox 'STBOX ZT((1,1,1,2000-01-01),(2,2,2,2000-01-02))', 5676);
SELECT setSRID(stbox 'STBOX T((,2000-01-01),(,2000-01-02))', 5676);
SELECT setSRID(stbox 'GEODSTBOX((1,1,1),(2,2,2))', 4326);
SELECT setSRID(stbox 'GEODSTBOX T((1,1,1,2000-01-01),(2,2,2,2000-01-02))', 4326);
SELECT setSRID(stbox 'GEODSTBOX T((,2000-01-01),(,2000-01-02))', 4326);

-- Tests independent of PROJ version
SELECT setPrecision(transform(transform(stbox 'SRID=4326;STBOX((1,1),(2,2))', 5676), 4326), 1);
SELECT setPrecision(transform(transform(stbox 'SRID=4326;STBOX T((1,1,2000-01-01),(2,2,2000-01-02))', 5676), 4326), 1);
SELECT setPrecision(transform(transform(stbox 'SRID=4326;STBOX Z((1,1,1),(2,2,2))', 5676), 4326), 1);
SELECT setPrecision(transform(transform(stbox 'SRID=4326;STBOX ZT((1,1,1,2000-01-01),(2,2,2,2000-01-02))', 5676), 4326), 1);
SELECT setPrecision(transform(transform(stbox 'SRID=4326;GEODSTBOX((1,1,1),(2,2,2))', 4269), 4326), 1);
SELECT setPrecision(transform(transform(stbox 'SRID=4326;GEODSTBOX T((1,1,1,2000-01-01),(2,2,2,2000-01-02))', 4269), 4326), 1);

SELECT DISTINCT SRID(b) FROM tbl_stbox;
SELECT MIN(xmin(setSRID(b,4326))) FROM tbl_stbox;

SELECT ROUND(MIN(xmin(transform(transform(setSRID(b,3812), 5676), 3812)))::numeric, 1) FROM tbl_stbox;
SELECT MIN(xmin(setPrecision(transform(transform(setSRID(b, 3812), 5676), 3812), 1))) FROM tbl_stbox;

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

SELECT startValue(transform(setSRID(tgeompoint 'Point(1 1 1)@2000-01-01', 5676), 4326)) = st_transform(geometry 'SRID=5676;Point(1 1 1)', 4326);
SELECT startValue(transform(setSRID(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 5676), 4326)) = st_transform(geometry 'SRID=5676;Point(1 1 1)', 4326);
SELECT startValue(transform(setSRID(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 5676), 4326)) = st_transform(geometry 'SRID=5676;Point(1 1 1)', 4326);
SELECT startValue(transform(setSRID(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 5676), 4326)) = st_transform(geometry 'SRID=5676;Point(1 1 1)', 4326);

--------------------------------------------------------

-- Temporal type
SELECT asEWKT(setPrecision(transform_gk(tgeompoint 'Point(13.43593 52.41721)@2018-12-20'), 6));
SELECT asEWKT(setPrecision(transform_gk(tgeompoint '{Point(13.43593 52.41721)@2018-12-20 10:00:00, Point(13.43605 52.41723)@2018-12-20 10:01:00}'), 6));
SELECT asEWKT(setPrecision(transform_gk(tgeompoint '[Point(13.43593 52.41721)@2018-12-20 10:00:00, Point(13.43605 52.41723)@2018-12-20 10:01:00]'), 6));
SELECT asEWKT(setPrecision(transform_gk(tgeompoint '{[Point(13.43593 52.41721)@2018-12-20 10:00:00, Point(13.43605 52.41723)@2018-12-20 10:01:00],[Point(13.43705 52.41724)@2018-12-20 10:02:00,Point(13.43805 52.41730)@2018-12-20 10:03:00]}'), 6));

-- PostGIS geometry
SELECT ST_AsText(setPrecision(transform_gk(geometry 'Point Empty'), 6));
SELECT ST_AsText(setPrecision(transform_gk(geometry 'Point(13.43593 52.41721)'), 6));
SELECT ST_AsText(setPrecision(geometry 'Linestring empty', 6));
SELECT ST_AsText(setPrecision(transform_gk(geometry 'Linestring(13.43593 52.41721,13.43593 52.41723)'), 6));

/* Error */
SELECT transform_gk(setPrecision(geometry 'Polygon((0 0,0 10,10 10,10 0,0 0))', 6));

--------------------------------------------------------

-- 2D
SELECT ST_AsText(setPrecision(geometry 'Point(1.123456789 1.123456789)', 6));
SELECT ST_AsText(setPrecision(geometry 'Linestring(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789)', 6));
SELECT ST_AsText(setPrecision(geometry 'Triangle((1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 1.123456789,1.123456789 1.123456789))', 6));
SELECT ST_AsText(setPrecision(geometry 'Circularstring(1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789)', 6));
SELECT ST_AsText(setPrecision(geometry 'Polygon((1.123456789 1.123456789,4.123456789 4.123456789,7.123456789 1.123456789,1.123456789 1.123456789),(3.123456789 2.123456789,4.123456789 3.123456789,5.123456789 2.123456789,3.123456789 2.123456789))', 6));
SELECT ST_AsText(setPrecision(geometry 'MultiPoint((1.123456789 1.123456789),(2.123456789 2.123456789),(3.123456789 3.123456789))', 6));
SELECT ST_AsText(setPrecision(geometry 'MultiLinestring((1.123456789 1.123456789,2.123456789 2.123456789,3.123456789 3.123456789),(4.123456789 4.123456789,5.123456789 5.123456789))', 6));
SELECT ST_AsText(setPrecision(geometry 'MultiPolygon(((1.123456789 1.123456789,4.123456789 4.123456789,7.123456789 1.123456789,1.123456789 1.123456789)),((3.123456789 2.123456789,4.123456789 3.123456789,5.123456789 2.123456789,3.123456789 2.123456789)))', 6));

SELECT ST_AsText(setPrecision(geometry 'Point Empty', 6));
SELECT ST_AsText(setPrecision(geometry 'Linestring Empty', 6));
SELECT ST_AsText(setPrecision(geometry 'Triangle Empty', 6));
SELECT ST_AsText(setPrecision(geometry 'Circularstring Empty', 6));
SELECT ST_AsText(setPrecision(geometry 'Polygon Empty', 6));
SELECT ST_AsText(setPrecision(geometry 'MultiPoint Empty', 6));
SELECT ST_AsText(setPrecision(geometry 'MultiLinestring Empty', 6));
SELECT ST_AsText(setPrecision(geometry 'MultiPolygon Empty', 6));
-- 3D
SELECT ST_AsText(setPrecision(geometry 'Point Z(1.123456789 1.123456789 1.123456789)', 6));
SELECT ST_AsText(setPrecision(geometry 'Linestring Z(1.123456789 1.123456789 1.123456789,2.123456789 2.123456789 2.123456789,3.123456789 3.123456789 3.123456789)', 6));
SELECT ST_AsText(setPrecision(geometry 'Triangle Z((1.123456789 1.123456789 1.123456789,2.123456789 2.123456789 2.123456789,3.123456789 1.123456789 1.123456789,1.123456789 1.123456789 1.123456789))', 6));
SELECT ST_AsText(setPrecision(geometry 'Circularstring Z(1.123456789 1.123456789 1.123456789,2.123456789 2.123456789 2.123456789,3.123456789 3.123456789 3.123456789)', 6));
SELECT ST_AsText(setPrecision(geometry 'Polygon Z((1.123456789 1.123456789 1.123456789,4.123456789 4.123456789 4.123456789,7.123456789 1.123456789 1.123456789,1.123456789 1.123456789 1.123456789),(3.123456789 2.123456789 2.123456789,4.123456789 3.123456789 3.123456789,5.123456789 2.123456789 2.123456789,3.123456789 2.123456789 2.123456789))', 6));
SELECT ST_AsText(setPrecision(geometry 'MultiPoint Z(1.123456789 1.123456789 1.123456789,2.123456789 2.123456789 2.123456789,3.123456789 3.123456789 3.123456789)', 6));
SELECT ST_AsText(setPrecision(geometry 'MultiLinestring Z((1.123456789 1.123456789 1.123456789,2.123456789 2.123456789 2.123456789,3.123456789 3.123456789 3.123456789),(4.123456789 4.123456789 4.123456789,5.123456789 5.123456789 5.123456789))', 6));
SELECT ST_AsText(setPrecision(geometry 'MultiPolygon Z(((1.123456789 1.123456789 1.123456789,4.123456789 4.123456789 4.123456789,7.123456789 1.123456789 1.123456789,1.123456789 1.123456789 1.123456789)),((3.123456789 2.123456789 2.123456789,4.123456789 3.123456789 3.123456789,5.123456789 2.123456789 2.123456789,3.123456789 2.123456789 2.123456789)))', 6));

--------------------------------------------------------

-- 2D
SELECT asText(setPrecision(tgeompoint 'Point(1.12345 1.12345)@2000-01-01', 2));
SELECT asText(setPrecision(tgeompoint '{Point(1.12345 1.12345)@2000-01-01, Point(2 2)@2000-01-02, Point(1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(setPrecision(tgeompoint '[Point(1.12345 1.12345)@2000-01-01, Point(2 2)@2000-01-02, Point(1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(setPrecision(tgeompoint '{[Point(1.12345 1.12345)@2000-01-01, Point(2 2)@2000-01-02, Point(1.12345 1.12345)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2));
SELECT asText(setPrecision(tgeogpoint 'Point(1.12345 1.12345)@2000-01-01', 2));
SELECT asText(setPrecision(tgeogpoint '{Point(1.12345 1.12345)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(setPrecision(tgeogpoint '[Point(1.12345 1.12345)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(setPrecision(tgeogpoint '{[Point(1.12345 1.12345)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.12345 1.12345)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 2));
-- 3D
SELECT asText(setPrecision(tgeompoint 'Point(1.12345 1.12345 1.12345)@2000-01-01', 2));
SELECT asText(setPrecision(tgeompoint '{Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(setPrecision(tgeompoint '[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(setPrecision(tgeompoint '{[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2));
SELECT asText(setPrecision(tgeogpoint 'Point(1.12345 1.12345 1.12345)@2000-01-01', 2));
SELECT asText(setPrecision(tgeogpoint '{Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03}', 2));
SELECT asText(setPrecision(tgeogpoint '[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03]', 2));
SELECT asText(setPrecision(tgeogpoint '{[Point(1.12345 1.12345 1.12345)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.12345 1.12345 1.12345)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', 2));

--------------------------------------------------------

SELECT asEWKT(tgeompoint 'Point(1 1)@2000-01-01'::tgeogpoint);
SELECT asEWKT(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'::tgeogpoint);
SELECT asEWKT(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'::tgeogpoint);
SELECT asEWKT(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'::tgeogpoint);
SELECT asEWKT(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'::tgeogpoint);
SELECT asEWKT(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'::tgeogpoint);

SELECT asEWKT(tgeogpoint 'Point(1.5 1.5)@2000-01-01'::tgeompoint);
SELECT asEWKT(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'::tgeompoint);
SELECT asEWKT(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'::tgeompoint);
SELECT asEWKT(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'::tgeompoint);
SELECT asEWKT(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'::tgeompoint);
SELECT asEWKT(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'::tgeompoint);

--------------------------------------------------------
--2D
SELECT getX(tgeompoint 'Point(1 1)@2000-01-01');
SELECT getX(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT getX(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT getX(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT getX(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT getX(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT getX(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT getX(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT getX(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT getX(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
SELECT getX(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT getX(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT getY(tgeompoint 'Point(1 1)@2000-01-01');
SELECT getY(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT getY(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT getY(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT getY(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT getY(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT getY(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT getY(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT getY(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT getY(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
SELECT getY(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT getY(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

/* Errors */
SELECT getZ(tgeompoint 'Point(1 1)@2000-01-01');
SELECT getZ(tgeogpoint 'Point(1.5 1.5)@2000-01-01');

-- 3D
SELECT getX(tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT getX(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT getX(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT getX(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT getX(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT getX(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT getX(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT getX(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT getX(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT getX(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');
SELECT getX(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT getX(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

SELECT getY(tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT getY(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT getY(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT getY(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT getY(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT getY(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT getY(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT getY(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT getY(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT getY(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');
SELECT getY(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT getY(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

SELECT getZ(tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT getZ(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT getZ(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT getZ(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT getZ(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT getZ(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');

SELECT getZ(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT getZ(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT getZ(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT getZ(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');
SELECT getZ(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT getZ(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

--------------------------------------------------------

-- 2D
SELECT ST_AsText(trajectory(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT ST_AsText(trajectory(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT ST_AsText(trajectory(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsText(trajectory(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsText(trajectory(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT ST_AsText(trajectory(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT ST_AsText(trajectory(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
SELECT ST_AsText(trajectory(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
-- 3D
SELECT ST_AsText(trajectory(tgeompoint 'Point(1 1 1)@2000-01-01'));
SELECT ST_AsText(trajectory(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'));
SELECT ST_AsText(trajectory(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'));
SELECT ST_AsText(trajectory(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01'));
SELECT ST_AsText(trajectory(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'));
SELECT ST_AsText(trajectory(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'));
SELECT ST_AsText(trajectory(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'));

SELECT ST_AsText(trajectory(tgeompoint '{[Point(1 1)@2001-01-01], [Point(1 1)@2001-02-01], [Point(1 1)@2001-03-01]}'));
SELECT ST_AsText(trajectory(tgeogpoint '{[Point(1 1)@2001-01-01], [Point(1 1)@2001-02-01], [Point(1 1)@2001-03-01]}'));
SELECT ST_AsText(trajectory(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02],[Point(1 1)@2000-01-03, Point(2 2)@2000-01-04]}'));

--------------------------------------------------------

-- 2D
SELECT round(length(tgeompoint 'Point(1 1)@2000-01-01')::numeric, 6);
SELECT round(length(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}')::numeric, 6);
SELECT round(length(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeogpoint 'Point(1.5 1.5)@2000-01-01')::numeric, 6);
SELECT round(length(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}')::numeric, 6);
SELECT round(length(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')::numeric, 6);
-- 3D
SELECT round(length(tgeompoint 'Point(1 1 1)@2000-01-01')::numeric, 6);
SELECT round(length(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}')::numeric, 6);
SELECT round(length(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01')::numeric, 6);
SELECT round(length(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}')::numeric, 6);
SELECT round(length(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);
SELECT round(length(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')::numeric, 6);
SELECT round(length(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')::numeric, 6);

-- 2D
SELECT round(cumulativeLength(tgeompoint 'Point(1 1)@2000-01-01'), 6);
SELECT round(cumulativeLength(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'), 6);
SELECT round(cumulativeLength(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Point(1.5 1.5)@2000-01-01'), 6);
SELECT round(cumulativeLength(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'), 6);
SELECT round(cumulativeLength(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'), 6);
-- 3D
SELECT round(cumulativeLength(tgeompoint 'Point(1 1 1)@2000-01-01'), 6);
SELECT round(cumulativeLength(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'), 6);
SELECT round(cumulativeLength(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01'), 6);
SELECT round(cumulativeLength(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'), 6);
SELECT round(cumulativeLength(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'), 6);

-- 2D
SELECT round(speed(tgeompoint 'Point(1 1)@2000-01-01'), 6);
SELECT round(speed(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'), 6);
SELECT round(speed(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round(speed(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round(speed(tgeogpoint 'Point(1.5 1.5)@2000-01-01'), 6);
SELECT round(speed(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'), 6);
SELECT round(speed(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'), 6);
SELECT round(speed(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'), 6);
/* Errors */
SELECT round(speed(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6);
SELECT round(speed(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6);
SELECT round(speed(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'), 6);
SELECT round(speed(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'), 6);
-- 3D
SELECT round(speed(tgeompoint 'Point(1 1 1)@2000-01-01'), 6);
SELECT round(speed(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'), 6);
SELECT round(speed(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round(speed(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(speed(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01'), 6);
SELECT round(speed(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}'), 6);
SELECT round(speed(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'), 6);
SELECT round(speed(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'), 6);
/* Errors */
SELECT round(speed(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6);
SELECT round(speed(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6);
SELECT round(speed(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]'), 6);
SELECT round(speed(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}'), 6);

-- 2D
SELECT ST_AsText(setPrecision(twcentroid(tgeompoint 'Point(1 1)@2000-01-01'), 6));
SELECT ST_AsText(setPrecision(twcentroid(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'), 6));
SELECT ST_AsText(setPrecision(twcentroid(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6));
SELECT ST_AsText(setPrecision(twcentroid(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6));
SELECT ST_AsText(setPrecision(twcentroid(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'), 6));
SELECT ST_AsText(setPrecision(twcentroid(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'), 6));
-- 3D
SELECT ST_AsText(setPrecision(twcentroid(tgeompoint 'Point(1 1 1)@2000-01-01'), 6));
SELECT ST_AsText(setPrecision(twcentroid(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}'), 6));
SELECT ST_AsText(setPrecision(twcentroid(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6));
SELECT ST_AsText(setPrecision(twcentroid(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6));
SELECT ST_AsText(setPrecision(twcentroid(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]'), 6));
SELECT ST_AsText(setPrecision(twcentroid(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}'), 6));

-- 2D
SELECT round(degrees(azimuth(tgeompoint 'Point(1 1)@2000-01-01')), 6);
SELECT round(degrees(azimuth(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}')), 6);
SELECT round(degrees(azimuth(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')), 6);
SELECT round(degrees(azimuth(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}')), 6);
-- Return negative result in PostGIS 2.5.5
-- SELECT round(degrees(azimuth(tgeogpoint 'Point(1.5 1.5)@2000-01-01')), 6);
-- SELECT round(degrees(azimuth(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}')), 6);
-- SELECT round(degrees(azimuth(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')), 6);
-- SELECT round(degrees(azimuth(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);
-- SELECT round(degrees(azimuth(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')), 6);
-- SELECT round(degrees(azimuth(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);
-- 3D
SELECT round(degrees(azimuth(tgeompoint 'Point(1 1 1)@2000-01-01')), 6);
SELECT round(degrees(azimuth(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}')), 6);
SELECT round(degrees(azimuth(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')), 6);
SELECT round(degrees(azimuth(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]')), 6);
SELECT round(degrees(azimuth(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}')), 6);

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
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6) IS NOT NULL;
-- 3D
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint 'Interp=Stepwise;[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]')), 6) IS NOT NULL;
SELECT 1 WHERE round(degrees(azimuth(tgeogpoint 'Interp=Stepwise;{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6) IS NOT NULL;

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

SELECT round(degrees(bearing(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint 'Interp=Stepwise;[Point(2 2)@2000-01-01, Point(1 1)@2000-01-02]')), 6);

SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint 'Point(2.5 2.5)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint 'Point(2.5 2.5)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint 'Point(2.5 2.5)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(2.5 2.5)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '{Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);
SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);
SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);
SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint '{[Point(2.5 2.5)@2000-01-01, Point(1.5 1.5)@2000-01-02, Point(2.5 2.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}')), 6);

SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(2.5 2.5 2.5)@2000-01-01')), 6);
SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '{Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03}')), 6);
SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03]')), 6);
SELECT round(degrees(bearing(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6);
SELECT round(degrees(bearing(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6);
SELECT round(degrees(bearing(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6);
SELECT round(degrees(bearing(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint '{[Point(2.5 2.5 2.5)@2000-01-01, Point(1.5 1.5 1.5)@2000-01-02, Point(2.5 2.5 2.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}')), 6);

--------------------------------------------------------

SELECT isSimple(tgeompoint '{Point(0 0)@2000-01-01}');
SELECT isSimple(tgeompoint '{Point(0 0)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT isSimple(tgeompoint '{Point(0 0)@2000-01-01, Point(0 0)@2000-01-02}');
SELECT isSimple(tgeompoint 'Interp=Stepwise;[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT isSimple(tgeompoint 'Interp=Stepwise;[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(2 0)@2000-01-03]');
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

--------------------------------------------------------

-- 2D
SELECT asText(atGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint '[Point(0 3)@2000-01-01, Point(1 1)@2000-01-02, Point(3 2)@2000-01-03, Point(0 3)@2000-01-04]', geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))'));
SELECT astext(atGeometry(tgeompoint '[Point(0 0)@2000-01-01, Point(3 3)@2000-01-02, Point(0 3)@2000-01-03, Point(
3 0)@2000-01-04]', 'Polygon((1 1,2 1,2 2,1 2,1 1))'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;[Point(0 3)@2000-01-01, Point(1 1)@2000-01-02, Point(3 2)@2000-01-03, Point(0 3)@2000-01-04]', geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))'));

SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;[Point(4 4)@2000-01-01, Point(3 3)@2000-01-02]', geometry 'Polygon((0 0,0 3,3 3,3 0,0 0))'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;[Point(4 4)@2000-01-01, Point(5 5)@2000-01-02, Point(3 3)@2000-01-03]', geometry 'Polygon((00 0,0 3,3 3,3 0,0 0))'));

SELECT asText(atGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));

-- 3D
SELECT asText(atGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(atGeometry(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(atGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring Z empty'));
SELECT asText(atGeometry(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring Z empty'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty'));
SELECT asText(atGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty'));

SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01]', geometry 'Linestring(0 0,1 1)'));
SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02]','Point(2 2)'));
SELECT asText(atGeometry(tgeompoint '[Point(0 1)@2000-01-01,Point(5 1)@2000-01-05]', geometry 'Linestring(0 0,2 2,3 1,4 1,5 0)'));
SELECT atGeometry(tgeompoint '[Point(0 0)@2000-01-01]', geometry 'Polygon((0 1,1 2,2 1,1 0,0 1))');
SELECT atGeometry(tgeompoint '{[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02],[Point(0 0)@2000-01-03]}', geometry 'Polygon((0 1,1 2,2 1,1 0,0 1))');

-- NULL
SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2000-01-01]', geometry 'Linestring(2 2,3 3)'));
SELECT asText(atGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]}', geometry 'Linestring(0 1,1 2)'));
SELECT asText(atGeometry(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02)', geometry 'Linestring(1 1,2 2)'));

-- Mixed 2D/3D
SELECT asText(atGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)'));
SELECT asText(atGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring(1 1,2 2)'));

/* Errors */
SELECT atGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');

-- 2D
SELECT asText(minusGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring(0 0,3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Linestring empty'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Linestring empty'));
-- 3D
SELECT asText(minusGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(minusGeometry(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring(0 0 0,3 3 3)'));
SELECT asText(minusGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring Z empty'));
SELECT asText(minusGeometry(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Linestring Z empty'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Linestring Z empty'));
SELECT asText(minusGeometry(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Linestring Z empty'));

SELECT asText(minusGeometry(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02)', geometry 'Linestring(0 1,2 1)'));
SELECT asText(minusGeometry(tgeompoint '{[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02)}', geometry 'Linestring(0 1,2 1)'));
SELECT asText(minusGeometry(tgeompoint '[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02]','Point(2 2)'));
SELECT asText(minusGeometry(tgeompoint '{[Point(1 1)@2000-01-01, Point(3 3)@2000-01-02],[Point(3 3)@2000-01-03]}','Point(2 2)'));

-- Mixed 2D/3D
SELECT asText(minusGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1 1,2 2 2)'));
SELECT asText(minusGeometry(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Linestring(1 1,2 2)'));

/* Errors */
SELECT minusGeometry(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Linestring(1 1,2 2)');

--------------------------------------------------------

SELECT asText(atStbox(tgeompoint 'Point(1 1)@2000-01-01', 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))'));
SELECT asText(atStbox(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))'));
SELECT asText(atStbox(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))'));
SELECT asText(atStbox(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))'));
SELECT asText(atStbox(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))'));
SELECT asText(atStbox(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))'));

SELECT asText(atStbox(tgeompoint 'Point(1 1)@2000-01-01', 'STBOX((1,1),(2,2))'));
SELECT asText(atStbox(tgeompoint 'Point(1 1)@2000-01-01', 'STBOX T((,2000-01-01),(,2000-01-02))'));
SELECT asText(atStbox(tgeompoint '(Point(2 2)@2000-01-02, Point(3 3)@2000-01-03]', 'STBOX T((,2000-01-01),(,2000-01-02))'));

SELECT asText(minusStbox(tgeompoint 'Point(1 1)@2000-01-01', 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))'));
SELECT asText(minusStbox(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))'));
SELECT asText(minusStbox(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))'));
SELECT asText(minusStbox(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))'));
SELECT asText(minusStbox(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))'));
SELECT asText(minusStbox(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))'));

/* Errors */
SELECT asText(atStbox(tgeompoint 'SRID=4326;Point(1 1)@2000-01-01', 'GEODSTBOX T((1,1,1,2000-01-01),(2,2,2,2000-01-02))'));
SELECT asText(atStbox(tgeompoint 'SRID=5676;Point(1 1)@2000-01-01', 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))'));
SELECT asText(atStbox(tgeompoint 'Point(1 1)@2000-01-01', 'STBOX ZT((1,1,1,2000-01-01),(2,2,2,2000-01-02))'));

-------------------------------------------------------------------------------
