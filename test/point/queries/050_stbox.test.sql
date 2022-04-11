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
-- STbox
-------------------------------------------------------------------------------

SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))';
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))';
SELECT stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))';
SELECT stbox 'GEODSTBOX ZT((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))';
SELECT stbox 'GEODSTBOX T(( , , 2001-01-03), ( , , 2001-01-03))';

SELECT stbox 'STBOX ZT((5,6,7,2001-01-08), (1,2,3,2001-01-04))';
SELECT stbox 'SRID=4326;STBOX ZT((5,6,7,2001-01-08), (1,2,3,2001-01-04))';
SELECT stbox 'SRID=4326;GEODSTBOX T((5,6,7,2001-01-08), (1,2,3,2001-01-04))';

/* Errors */
SELECT stbox 'AAA(1, 2, 3)';
SELECT stbox 'stbox(1, 2, 3)';
SELECT stbox 'stbox((AA, 2, 3))';
SELECT stbox 'stbox((1, AA, 3))';
SELECT stbox 'stbox((,),(,))';
SELECT stbox 'stbox z((1, 2, AA))';
SELECT stbox 'stbox t((1, 2, AA))';
SELECT stbox 'stbox((1, 2, 3))';
SELECT stbox 'stbox t((1, 2, 2001-01-03))';
SELECT stbox 'stbox t((1, 2, 2001-01-03),()';
SELECT stbox 'stbox t((1, 2, 2001-01-03),(1)';
SELECT stbox 'stbox z((1, 2, 3),(1,2)';
SELECT stbox 'stbox t((1, 2, 2001-01-03),(1,2)';
SELECT stbox 'stbox t((1, 2, 2001-01-03),(1,2,2001-01-03)';
SELECT stbox 'SRID=4326;STBOX T((,2001-01-08), (,2001-01-04))';
SELECT stbox 'SRID=4326;GEODSTBOX T((,2001-01-08), (,2001-01-04))';

-- Send/receive functions

COPY tbl_stbox3d TO '/tmp/tbl_stbox3d' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_stbox3d_tmp;
CREATE TABLE tbl_stbox3d_tmp AS TABLE tbl_stbox3d WITH NO DATA;
COPY tbl_stbox3d_tmp FROM '/tmp/tbl_stbox3d' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_stbox3d t1, tbl_stbox3d_tmp t2 WHERE t1.k = t2.k AND t1.b <> t2.b;
DROP TABLE tbl_stbox3d_tmp;

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT stbox_t('2001-01-03','2001-01-06');
SELECT stbox(1,2,3,4);
SELECT stbox_z(1,2,3,4,5,6);
SELECT stbox_t(1,2,'2001-01-03',4,5,'2001-01-06');
SELECT stbox_zt(1,2,3,'2001-01-04',5,6,7,'2001-01-08');

SELECT geodstbox_t('2001-01-03','2001-01-06');
SELECT geodstbox(1,2,3,4,5,6);
SELECT geodstbox_z(1,2,3,4,5,6);
SELECT geodstbox_t(1,2,3,'2001-01-04',5,6,7,'2001-01-08');
SELECT geodstbox_zt(1,2,3,'2001-01-03',4,5,6,'2001-01-06');

SELECT geodstbox_t('2001-01-04','2001-01-08');
SELECT geodstbox(1,2,3,4,5,6);
SELECT geodstbox_t(1,2,3,'2001-01-04',5,6,7,'2001-01-08');

SELECT stbox_zt(8,7,6,'2001-01-05',4,3,2,'2001-01-01');
SELECT stbox_t(6,5,'2001-01-04',3,2,'2001-01-01');
SELECT geodstbox_zt(8,7,6,'2001-01-05',4,3,2,'2001-01-01');

-------------------------------------------------------------------------------
-- Casting
-------------------------------------------------------------------------------

SELECT stbox 'STBOX T((1.0, 2.0, 2000-01-01), (3.0, 4.0, 2000-01-02))'::period;
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))'::period;
SELECT stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))'::period;
/* Errors */
SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))'::period;
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))'::period;

SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))'::box2d;
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))'::box2d;
SELECT stbox 'STBOX T((1.0, 2.0, 2000-01-01), (3.0, 4.0, 2000-01-02))'::box2d;
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))'::box2d;
/* Errors */
SELECT stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))'::box2d;

SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))'::box3d;
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))'::box3d;
SELECT stbox 'STBOX T((1.0, 2.0, 2000-01-01), (3.0, 4.0, 2000-01-02))'::box3d;
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))'::box3d;
/* Errors */
SELECT stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))'::box3d;

SELECT ST_AsEWKT(stbox 'SRID=4326;STBOX T((1,1,2000-01-01),(5,5,2000-01-05))'::geometry);
SELECT ST_AsEWKT(stbox 'SRID=4326;STBOX T((1,1,2000-01-01),(1,5,2000-01-05))'::geometry);
SELECT ST_AsEWKT(stbox 'SRID=4326;STBOX T((1,1,2000-01-01),(5,1,2000-01-05))'::geometry);
SELECT ST_AsEWKT(stbox 'SRID=4326;STBOX T((1,1,2000-01-01),(1,1,2000-01-05))'::geometry);
SELECT ST_AsEWKT(stbox 'SRID=4326;STBOX ZT((1,1,1,2000-01-01),(5,5,5,2000-01-05))'::geometry);
SELECT ST_AsEWKT(stbox 'SRID=4326;STBOX ZT((1,1,1,2000-01-01),(1,5,5,2000-01-05))'::geometry);
SELECT ST_AsEWKT(stbox 'SRID=4326;STBOX ZT((1,1,1,2000-01-01),(5,1,5,2000-01-05))'::geometry);
SELECT ST_AsEWKT(stbox 'SRID=4326;STBOX ZT((1,1,1,2000-01-01),(5,5,1,2000-01-05))'::geometry);
SELECT ST_AsEWKT(stbox 'SRID=4326;STBOX ZT((1,1,1,2000-01-01),(1,1,5,2000-01-05))'::geometry);
SELECT ST_AsEWKT(stbox 'SRID=4326;STBOX ZT((1,1,1,2000-01-01),(1,5,1,2000-01-05))'::geometry);
SELECT ST_AsEWKT(stbox 'SRID=4326;STBOX ZT((1,1,1,2000-01-01),(5,1,1,2000-01-05))'::geometry);
SELECT ST_AsEWKT(stbox 'SRID=4326;STBOX ZT((1,1,1,2000-01-01),(1,1,1,2000-01-05))'::geometry);
/* Errors */
SELECT stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))'::geometry;

-------------------------------------------------------------------------------

SELECT (geometry 'Polygon((1 1,1 2,2 2,2 1,1 1))'::box2d)::stbox;
SELECT (geometry 'Polygon((1 1 1,1 2 2,2 2 2,2 1 1,1 1 1))'::box3d)::stbox;

-------------------------------------------------------------------------------

SELECT MAX(duration(b::period)) FROM tbl_stbox;
SELECT MAX(ST_XMax(b::box2d)) FROM tbl_stbox;
SELECT MAX(ST_XMax(b::box3d)) FROM tbl_stbox;

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT hasx(stbox 'STBOX((1.0, 2.0), (3.0, 4.0))');
SELECT hasz(stbox 'STBOX((1.0, 2.0), (3.0, 4.0))');
SELECT hast(stbox 'STBOX((1.0, 2.0), (3.0, 4.0))');
SELECT isGeodetic(stbox 'STBOX((1.0, 2.0), (3.0, 4.0))');

SELECT xmin(stbox 'STBOX((1.0, 2.0), (3.0, 4.0))');
SELECT ymin(stbox 'STBOX((1.0, 2.0), (3.0, 4.0))');
SELECT zmin(stbox 'STBOX((1.0, 2.0), (3.0, 4.0))');
SELECT tmin(stbox 'STBOX((1.0, 2.0), (3.0, 4.0))');
SELECT xmax(stbox 'STBOX((1.0, 2.0), (3.0, 4.0))');
SELECT ymax(stbox 'STBOX((1.0, 2.0), (3.0, 4.0))');
SELECT zmax(stbox 'STBOX((1.0, 2.0), (3.0, 4.0))');
SELECT tmax(stbox 'STBOX((1.0, 2.0), (3.0, 4.0))');

SELECT xmin(stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))');
SELECT ymin(stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))');
SELECT zmin(stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))');
SELECT tmin(stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))');
SELECT xmax(stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))');
SELECT ymax(stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))');
SELECT zmax(stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))');
SELECT tmax(stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))');

SELECT xmin(stbox 'STBOX T((1.0, 2.0, 2000-01-01), (3.0, 4.0, 2000-01-02))');
SELECT ymin(stbox 'STBOX T((1.0, 2.0, 2000-01-01), (3.0, 4.0, 2000-01-02))');
SELECT zmin(stbox 'STBOX T((1.0, 2.0, 2000-01-01), (3.0, 4.0, 2000-01-02))');
SELECT tmin(stbox 'STBOX T((1.0, 2.0, 2000-01-01), (3.0, 4.0, 2000-01-02))');
SELECT xmax(stbox 'STBOX T((1.0, 2.0, 2000-01-01), (3.0, 4.0, 2000-01-02))');
SELECT ymax(stbox 'STBOX T((1.0, 2.0, 2000-01-01), (3.0, 4.0, 2000-01-02))');
SELECT zmax(stbox 'STBOX T((1.0, 2.0, 2000-01-01), (3.0, 4.0, 2000-01-02))');
SELECT tmax(stbox 'STBOX T((1.0, 2.0, 2000-01-01), (3.0, 4.0, 2000-01-02))');

SELECT xmin(stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))');
SELECT ymin(stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))');
SELECT zmin(stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))');
SELECT tmin(stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))');
SELECT xmax(stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))');
SELECT ymax(stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))');
SELECT zmax(stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))');
SELECT tmax(stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))');

SELECT xmin(stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))');
SELECT ymin(stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))');
SELECT zmin(stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))');
SELECT tmin(stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))');
SELECT xmax(stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))');
SELECT ymax(stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))');
SELECT zmax(stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))');
SELECT tmax(stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))');

SELECT round(stbox 'STBOX((1.12345,1.12345),(2.12345,2.12345))', 2);
SELECT round(stbox 'STBOX T((1.12345,1.12345,2000-01-01),(2.12345,2.12345,2000-01-02))', 2);
SELECT round(stbox 'STBOX Z((1.12345,1.12345,1.12345),(2.12345,2.12345,2.12345))', 2);
SELECT round(stbox 'STBOX ZT((1.12345,1.12345,1.12345,2000-01-01),(2.12345,2.12345,2.12345,2000-01-02))', 2);
SELECT round(stbox 'GEODSTBOX((1.12345,1.12345,1.12345),(2.12345,2.12345,2.12345))', 2);
SELECT round(stbox 'GEODSTBOX T((1.12345,1.12345,1.12345,2000-01-01),(2.12345,2.12345,2.12345,2000-01-02))', 2);
/* Errors */
SELECT round(stbox 'STBOX T((,2000-01-01),(,2000-01-02))', 2);
SELECT round(stbox 'GEODSTBOX T((,2000-01-01),(,2000-01-02))', 2);

-------------------------------------------------------------------------------

SELECT MIN(xmin(b)) FROM tbl_stbox;
SELECT MAX(xmax(b)) FROM tbl_stbox;
SELECT MIN(ymin(b)) FROM tbl_stbox;
SELECT MAX(ymax(b)) FROM tbl_stbox;
SELECT MIN(zmin(b)) FROM tbl_stbox;
SELECT MAX(zmax(b)) FROM tbl_stbox;
SELECT MIN(tmin(b)) FROM tbl_stbox;
SELECT MAX(tmax(b)) FROM tbl_stbox;

-------------------------------------------------------------------------------
-- Topological operators
-------------------------------------------------------------------------------

SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' && stbox 'STBOX T((1.0, 2.0, 2000-01-01), (1.0, 2.0, 2000-01-01))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' @> stbox 'STBOX T((1.0, 2.0, 2000-01-01), (1.0, 2.0, 2000-01-01))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' <@ stbox 'STBOX T((1.0, 2.0, 2000-01-01), (1.0, 2.0, 2000-01-01))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' -|- stbox 'STBOX T((1.0, 2.0, 2000-01-01), (1.0, 2.0, 2000-01-01))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' ~= stbox 'STBOX T((1.0, 2.0, 2000-01-01), (1.0, 2.0, 2000-01-01))';
SELECT stbox 'STBOX Z((1.0, 1.0, 1.0), (2.0, 2.0, 2.0))' ~= stbox 'STBOX Z((1.0, 1.0, 1.0), (2.0, 2.0, 3.0))';

SELECT period '[2000-01-01, 2000-01-02]'::stbox -|- period '[2000-01-02, 2000-01-03]'::stbox;

/* Errors */
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' && stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' @> stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' <@ stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' -|- stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' ~= stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' && stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' @> stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' <@ stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' -|- stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' ~= stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b && t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b @> t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b <@ t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b ~= t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b -|- t2.b;

-------------------------------------------------------------------------------
-- Position operators
-------------------------------------------------------------------------------

SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' << stbox 'STBOX T((1.0, 2.0, 2001-01-01), (1.0, 2.0, 2001-01-01))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' &< stbox 'STBOX T((1.0, 2.0, 2001-01-01), (1.0, 2.0, 2001-01-01))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' >> stbox 'STBOX T((1.0, 2.0, 2001-01-01), (1.0, 2.0, 2001-01-01))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' &> stbox 'STBOX T((1.0, 2.0, 2001-01-01), (1.0, 2.0, 2001-01-01))';

SELECT stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))' << stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))';
SELECT stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))' &< stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))';
SELECT stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))' >> stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))';
SELECT stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))' &> stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))';

SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' <<| stbox 'STBOX T((1.0, 2.0, 2001-01-03), (1.0, 2.0, 2001-01-03))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' &<| stbox 'STBOX T((1.0, 2.0, 2001-01-03), (1.0, 2.0, 2001-01-03))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' |>> stbox 'STBOX T((1.0, 2.0, 2001-01-03), (1.0, 2.0, 2001-01-03))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' |&> stbox 'STBOX T((1.0, 2.0, 2001-01-03), (1.0, 2.0, 2001-01-03))';

SELECT stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))' <<| stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))';
SELECT stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))' &<| stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))';
SELECT stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))' |>> stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))';
SELECT stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))' |&> stbox 'STBOX T((-inf,-inf,2001-01-01),(inf,inf,2001-01-02))';

SELECT stbox 'STBOX Z((1.0, 1.0, 1.0), (2.0, 2.0, 2.0))' <</ stbox 'STBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX Z((1.0, 1.0, 1.0), (2.0, 2.0, 2.0))' &</ stbox 'STBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX Z((1.0, 1.0, 1.0), (2.0, 2.0, 2.0))' />> stbox 'STBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX Z((1.0, 1.0, 1.0), (2.0, 2.0, 2.0))' /&> stbox 'STBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

SELECT stbox 'STBOX T((1.0, 1.0, 2001-01-01), (2.0, 2.0, 2001-01-02))' <<# stbox 'STBOX T((1.0, 2.0, 2001-01-01), (1.0, 2.0, 2001-01-01))';
SELECT stbox 'STBOX T((1.0, 1.0, 2001-01-01), (2.0, 2.0, 2001-01-02))' &<# stbox 'STBOX T((1.0, 2.0, 2001-01-01), (1.0, 2.0, 2001-01-01))';
SELECT stbox 'STBOX T((1.0, 1.0, 2001-01-01), (2.0, 2.0, 2001-01-02))' #>> stbox 'STBOX T((1.0, 2.0, 2001-01-01), (1.0, 2.0, 2001-01-01))';
SELECT stbox 'STBOX T((1.0, 1.0, 2001-01-01), (2.0, 2.0, 2001-01-02))' #&> stbox 'STBOX T((1.0, 2.0, 2001-01-01), (1.0, 2.0, 2001-01-01))';

SELECT stbox 'STBOX ZT((1.0, 1.0, 1.0, 2001-01-01), (2.0, 2.0, 2.0, 2001-01-02))' <<# stbox 'STBOX T((1.0, 2.0, 2001-01-03), (1.0, 2.0, 2001-01-03))';
SELECT stbox 'STBOX ZT((1.0, 1.0, 1.0, 2001-01-01), (2.0, 2.0, 2.0, 2001-01-02))' &<# stbox 'STBOX T((1.0, 2.0, 2001-01-03), (1.0, 2.0, 2001-01-03))';
SELECT stbox 'STBOX ZT((1.0, 1.0, 1.0, 2001-01-01), (2.0, 2.0, 2.0, 2001-01-02))' #>> stbox 'STBOX T((1.0, 2.0, 2001-01-03), (1.0, 2.0, 2001-01-03))';
SELECT stbox 'STBOX ZT((1.0, 1.0, 1.0, 2001-01-01), (2.0, 2.0, 2.0, 2001-01-02))' #&> stbox 'STBOX T((1.0, 2.0, 2001-01-03), (1.0, 2.0, 2001-01-03))';

/* Errors */
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' << stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' &< stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' >> stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' &> stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' <<| stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' &<| stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' |>> stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' |&> stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX Z((1.0, 1.0, 1.0), (2.0, 2.0, 2.0))' <</ stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX Z((1.0, 1.0, 1.0), (2.0, 2.0, 2.0))' &</ stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX Z((1.0, 1.0, 1.0), (2.0, 2.0, 2.0))' />> stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX Z((1.0, 1.0, 1.0), (2.0, 2.0, 2.0))' /&> stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b << t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b &< t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b >> t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b &> t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b <<| t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b &<| t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b |>> t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b |&> t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b <<# t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b &<# t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b #>> t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b #&> t2.b;
-- Errors
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b <</ t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b &</ t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b />> t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b /&> t2.b;

-------------------------------------------------------------------------------
-- Set operators
-------------------------------------------------------------------------------

SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))' + stbox 'STBOX((1.0, 2.0), (3.0, 4.0))';
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))' + stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))';
SELECT stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))' + stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' + stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))' + stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))';
/* Errors */
SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))' + stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))';
SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))' + stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))';
SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))' + stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))' + stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))';
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))' + stbox 'STBOX((1.0, 2.0), (3.0, 4.0))';
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))' + stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))';
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))' + stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))' + stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))';
SELECT stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))' + stbox 'STBOX((1.0, 2.0), (3.0, 4.0))';
SELECT stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))' + stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))';
SELECT stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))' + stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))' + stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' + stbox 'STBOX((1.0, 2.0), (3.0, 4.0))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' + stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' + stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' + stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))';
SELECT stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))' + stbox 'STBOX((1.0, 2.0), (3.0, 4.0))';
SELECT stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))' + stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))';
SELECT stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))' + stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))';
SELECT stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))' + stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' + stbox 'STBOX ZT((11.0, 2.0, 3.0, 2001-01-01), (14.0, 5.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' + stbox 'STBOX ZT((1.0, 12.0, 3.0, 2001-01-01), (4.0, 15.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' + stbox 'STBOX ZT((1.0, 2.0, 13.0, 2001-01-01), (4.0, 5.0, 16.0, 2001-01-02))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' + stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-02-01), (4.0, 5.0, 6.0, 2001-02-02))';

SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' + stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))' + stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))';
SELECT stbox 'GEODSTBOX T(( , , 2001-01-03), ( , , 2001-01-03))' + stbox 'GEODSTBOX T(( , , 2001-01-03), ( , , 2001-01-03))';
/* Errors */
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' + stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' + stbox 'GEODSTBOX T(( , , 2001-01-03), ( , , 2001-01-03))';
SELECT stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))' + stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))' + stbox 'GEODSTBOX T(( , , 2001-01-03), ( , , 2001-01-03))';
SELECT stbox 'GEODSTBOX T(( , , 2001-01-03), ( , , 2001-01-03))' + stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'GEODSTBOX T(( , , 2001-01-03), ( , , 2001-01-03))' + stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))';
SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))' + stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

-------------------------------------------------------------------------------

SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))' * stbox 'STBOX((1.0, 2.0), (3.0, 4.0))';
SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))' * stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))';
SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))' * stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))';
SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))' * stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))' * stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))';

SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))' * stbox 'STBOX((1.0, 2.0), (3.0, 4.0))';
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))' * stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))';
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))' * stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))';
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))' * stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))' * stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))';

SELECT stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))' * stbox 'STBOX((1.0, 2.0), (3.0, 4.0))';
SELECT stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))' * stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))';
SELECT stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))' * stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))';
SELECT stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))' * stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))' * stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))';

SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' * stbox 'STBOX((1.0, 2.0), (3.0, 4.0))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' * stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' * stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' * stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' * stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))';

SELECT stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))' * stbox 'STBOX((1.0, 2.0), (3.0, 4.0))';
SELECT stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))' * stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))';
SELECT stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))' * stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))';
SELECT stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))' * stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))' * stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))';

SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' * stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' * stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' * stbox 'GEODSTBOX T(( , , 2001-01-03), ( , , 2001-01-03))';

SELECT stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))' * stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))' * stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))';
SELECT stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))' * stbox 'GEODSTBOX T(( , , 2001-01-03), ( , , 2001-01-03))';

SELECT stbox 'GEODSTBOX T(( , , 2001-01-03), ( , , 2001-01-03))' * stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'GEODSTBOX T(( , , 2001-01-03), ( , , 2001-01-03))' * stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))';
SELECT stbox 'GEODSTBOX T(( , , 2001-01-03), ( , , 2001-01-03))' * stbox 'GEODSTBOX T(( , , 2001-01-03), ( , , 2001-01-03))';

SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' * stbox 'STBOX ZT((11.0, 2.0, 3.0, 2001-01-01), (14.0, 5.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' * stbox 'STBOX ZT((1.0, 12.0, 3.0, 2001-01-01), (4.0, 15.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' * stbox 'STBOX ZT((1.0, 2.0, 13.0, 2001-01-01), (4.0, 5.0, 16.0, 2001-01-02))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))' * stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-02-01), (4.0, 5.0, 6.0, 2001-02-02))';
/* Errors */
SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))' * stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

-------------------------------------------------------------------------------

SELECT MAX(xmax(t1.b + t2.b)) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b && t2.b;
SELECT MAX(xmax(t1.b * t2.b)) FROM tbl_stbox t1, tbl_stbox t2;

-------------------------------------------------------------------------------
-- Extent aggregation
-------------------------------------------------------------------------------

WITH test(box) AS (
  SELECT NULL::stbox UNION ALL SELECT stbox 'STBOX T((1,1,2000-01-01),(2,2,2000-01-02))' UNION ALL
  SELECT NULL::stbox UNION ALL SELECT stbox 'STBOX T((1,1,2000-01-01),(3,3,2000-01-03))' )
SELECT extent(box) FROM test;

-- encourage use of parallel plans
set parallel_setup_cost=0;
set parallel_tuple_cost=0;
set min_parallel_table_scan_size=0;
set max_parallel_workers_per_gather=2;

SELECT round(extent(temp::stbox),6) FROM tbl_tgeompoint3D_big;

-- reset to default values
reset parallel_setup_cost;
reset parallel_tuple_cost;
reset min_parallel_table_scan_size;
reset max_parallel_workers_per_gather;

-------------------------------------------------------------------------------
-- Comparison functions
-------------------------------------------------------------------------------

SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))', stbox 'STBOX ZT((2,2,3,2001-01-04), (2,2,3,2001-01-04))');
SELECT stbox_cmp(stbox 'STBOX ZT((2,2,3,2001-01-04), (2,2,3,2001-01-04))', stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))', stbox 'STBOX ZT((1,3,3,2001-01-04), (1,3,3,2001-01-04))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,3,3,2001-01-04), (1,3,3,2001-01-04))', stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))', stbox 'STBOX ZT((1,2,4,2001-01-04), (1,2,4,2001-01-04))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,4,2001-01-04), (1,2,4,2001-01-04))', stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))', stbox 'STBOX ZT((1,2,3,2001-01-05), (1,2,3,2001-01-05))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-05), (1,2,3,2001-01-05))', stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))', stbox 'STBOX ZT((1,2,3,2001-01-04), (2,2,3,2001-01-04))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (2,2,3,2001-01-04))', stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))', stbox 'STBOX ZT((1,2,3,2001-01-04), (1,3,3,2001-01-04))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (1,3,3,2001-01-04))', stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))', stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,4,2001-01-04))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,4,2001-01-04))', stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))', stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-05))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-05))', stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))', stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))');

SELECT stbox_cmp(stbox 'SRID=5676;STBOX((1 1),(2 2))', stbox 'STBOX((1 1),(2 2))');
SELECT stbox_cmp(stbox 'STBOX((1 1),(2 2))', stbox 'SRID=5676;STBOX((1 1),(2 2))');
SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))', stbox 'STBOX Z((1,2,3), (1,2,3))');
SELECT stbox_cmp(stbox 'STBOX Z((1,2,3), (1,2,3))', stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))');

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b = t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b <> t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b < t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b <= t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b > t2.b;
SELECT COUNT(*) FROM tbl_stbox t1, tbl_stbox t2 WHERE t1.b >= t2.b;

-------------------------------------------------------------------------------
