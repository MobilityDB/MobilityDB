-------------------------------------------------------------------------------
-- STbox
-------------------------------------------------------------------------------

SELECT stbox 'STBOX((1.0, 2.0), (3.0, 4.0))';
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))';
SELECT stbox 'STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02))';
SELECT stbox 'STBOX T(( , , 2001-01-01), ( , , 2001-01-02))';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))';
SELECT stbox 'GEODSTBOX T(( , , 2001-01-03), ( , , 2001-01-03))';

SELECT stbox 'STBOX ZT((5,6,7,2001-01-08), (1,2,3,2001-01-04))';
SELECT stbox 'SRID=4326;STBOX ZT((5,6,7,2001-01-08), (1,2,3,2001-01-04))';
SELECT stbox 'SRID=4326;GEODSTBOX T((5,6,7,2001-01-08), (1,2,3,2001-01-04))';

/* Errors */
SELECT stbox 'AAA(1, 2, 3)';
SELECT stbox 'stbox(1, 2, 3)';
SELECT stbox 'stbox((AA, 2, 3))';
SELECT stbox 'stbox((1, AA, 3))';
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

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT stboxt('2001-01-03','2001-01-06');
SELECT stbox(1,2,3,4);
SELECT stbox(1,2,3,4,5,6);
SELECT stbox(1,2,3,'2001-01-04',5,6,7,'2001-01-08');
SELECT stboxt(1,2,'2001-01-03',4,5,'2001-01-06');
SELECT geodstboxt('2001-01-04','2001-01-08');
SELECT geodstbox(1,2,3,4,5,6);
SELECT geodstbox(1,2,3,'2001-01-04',5,6,7,'2001-01-08');

SELECT stbox(8,7,6,'2001-01-05',4,3,2,'2001-01-01');
SELECT stboxt(6,5,'2001-01-04',3,2,'2001-01-01');
SELECT geodstbox(8,7,6,'2001-01-05',4,3,2,'2001-01-01');

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

-------------------------------------------------------------------------------

SELECT MAX(timespan(b::period)) FROM tbl_stbox;
SELECT MAX(ST_XMax(b::box2d)) FROM tbl_stbox;
SELECT MAX(ST_XMax(b::box3d)) FROM tbl_stbox;

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

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

SELECT srid(stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))');
SELECT srid(stbox 'SRID=4326;STBOX ZT((1.0, 2.0, 3.0, 2000-01-01), (4.0, 5.0, 6.0, 2000-01-02))');
SELECT srid(stbox 'STBOX T((, , 2000-01-01), (, , 2000-01-02))');

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

SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b && t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b @> t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b <@ t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b ~= t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b -|- t2.b;

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

SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b << t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b &< t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b >> t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b &> t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b <<| t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b &<| t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b |>> t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b |&> t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b <</ t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b &</ t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b />> t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b /&> t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b <<# t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b &<# t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b #>> t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b #&> t2.b;

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

SELECT stbox_cmp(stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))', stbox 'STBOX Z((1,2,3), (1,2,3))');
SELECT stbox_cmp(stbox 'STBOX Z((1,2,3), (1,2,3))', stbox 'STBOX ZT((1,2,3,2001-01-04), (1,2,3,2001-01-04))');

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b = t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b <> t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b < t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b <= t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b > t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b >= t2.b;

-------------------------------------------------------------------------------
