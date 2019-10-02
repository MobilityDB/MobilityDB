-------------------------------------------------------------------------------
-- STbox
-------------------------------------------------------------------------------

SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX T((1.0, 2.0, 2001-01-03), (1.0, 2.0, 2001-01-03))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-04))';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-04), (1.0, 2.0, 3.0, 2001-01-03))';
SELECT stbox 'STBOX T(( , , 2001-01-03), ( , , 2001-01-03))';

SELECT stbox 'STBOX ZT((5,6,7,2001-01-08), (1,2,3,2001-01-04))';

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

SELECT stbox(1,2,3,4);
SELECT stbox(1,2,3,4,5,6);
SELECT stbox(1,2,3,'2001-01-04',5,6,7,'2001-01-08');
SELECT stboxt(1,2,'2001-01-03',4,5,'2001-01-06');
SELECT geodstbox(1,2,3,4,5,6);
SELECT geodstbox(1,2,3,'2001-01-04',5,6,7,'2001-01-08');

SELECT stbox(8,7,6,'2001-01-05',4,3,2,'2001-01-01');
SELECT stboxt(6,5,'2001-01-04',3,2,'2001-01-01');
SELECT geodstbox(8,7,6,'2001-01-05',4,3,2,'2001-01-01');

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

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b = t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b <> t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b < t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b <= t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b > t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b >= t2.b;

SELECT count(*) FROM tbl_tgeompoint WHERE temp::stbox IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint WHERE temp::stbox IS NOT NULL;

SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b && t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b @> t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b <@ t2.b;
SELECT count(*) FROM tbl_stbox t1, tbl_stbox t2 where t1.b ~= t2.b;

-------------------------------------------------------------------------------
