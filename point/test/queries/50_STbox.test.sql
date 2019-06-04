/*****************************************************************************
 * STbox
 *****************************************************************************/

SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT stbox 'STBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX T((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))';
SELECT stbox 'STBOX T(( , , 3.0), ( , , 3.0))';

/* Errors */
SELECT stbox 'AAA(1, 2, 3)';
SELECT stbox 'stbox(1, 2, 3)';
SELECT stbox 'stbox((AA, 2, 3))';
SELECT stbox 'stbox((1, AA, 3))';
SELECT stbox 'stbox z((1, 2, AA))';
SELECT stbox 'stbox t((1, 2, AA))';
SELECT stbox 'stbox((1, 2, 3))';
SELECT stbox 'stbox t((1, 2, 3))';
SELECT stbox 'stbox t((1, 2, 3),()';
SELECT stbox 'stbox t((1, 2, 3),(1)'; 
SELECT stbox 'stbox z((1, 2, 3),(1,2)'; 
SELECT stbox 'stbox t((1, 2, 3),(1,2)'; 
SELECT stbox 'stbox t((1, 2, 3),(1,2,3)'; 

SELECT stbox(1,2,3,4);
SELECT stbox(1,2,3,4,5,6);
SELECT stbox(1,2,3,4,5,6,7,8);
SELECT stboxt(1,2,3,4,5,6);
SELECT geodstbox(1,2,3,4,5,6);
SELECT geodstbox(1,2,3,4,5,6,7,8);

SELECT stbox(8,7,6,5,4,3,2,1);
SELECT stboxt(6,5,4,3,2,1);
SELECT geodstbox(8,7,6,5,4,3,2,1);

SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', stbox 'STBOX ZT((2.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((2.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', stbox 'STBOX ZT((1.0, 3.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 3.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', stbox 'STBOX ZT((1.0, 2.0, 4.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 4.0, 4.0), (1.0, 2.0, 3.0, 4.0))', stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', stbox 'STBOX ZT((1.0, 2.0, 3.0, 5.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 3.0, 5.0), (1.0, 2.0, 3.0, 4.0))', stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (2.0, 2.0, 3.0, 4.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (2.0, 2.0, 3.0, 4.0))', stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 3.0, 3.0, 4.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 3.0, 3.0, 4.0))', stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 4.0, 4.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 4.0, 4.0))', stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 5.0))'); 
SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 5.0))', stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 

SELECT stbox_cmp(stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', stbox 'STBOX ZT((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 

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

/*****************************************************************************/