/*****************************************************************************
 * Gbox
 *****************************************************************************/

SELECT gbox 'GBOX((1.0, 2.0), (1.0, 2.0))';
SELECT gbox 'GBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT gbox 'GBOX M((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))';
SELECT gbox 'GEODBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT gbox 'GEODBOX M((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))';

/* Errors */
SELECT gbox 'AAA(1, 2, 3)';
SELECT gbox 'gbox(1, 2, 3)';
SELECT gbox 'gbox((AA, 2, 3))';
SELECT gbox 'gbox((1, AA, 3))';
SELECT gbox 'gbox z((1, 2, AA))';
SELECT gbox 'gbox m((1, 2, AA))';
SELECT gbox 'gbox((1, 2, 3))';
SELECT gbox 'gbox m((1, 2, 3))';
SELECT gbox 'gbox m((1, 2, 3),()';
SELECT gbox 'gbox m((1, 2, 3),(1)'; 
SELECT gbox 'gbox z((1, 2, 3),(1,2)'; 
SELECT gbox 'gbox m((1, 2, 3),(1,2)'; 
SELECT gbox 'gbox m((1, 2, 3),(1,2,3)'; 

SELECT gbox(1,2,3,4);
SELECT gbox(1,2,3,4,5,6);
SELECT gbox(1,2,3,4,5,6,7,8);
SELECT gbox3dm(1,2,3,4,5,6);
SELECT geodbox(1,2,3,4,5,6);
SELECT geodbox(1,2,3,4,5,6,7,8);

SELECT gbox(8,7,6,5,4,3,2,1);
SELECT gbox3dm(6,5,4,3,2,1);
SELECT geodbox(8,7,6,5,4,3,2,1);

SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', gbox 'GBOX ZM((2.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((2.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', gbox 'GBOX ZM((1.0, 3.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 3.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', gbox 'GBOX ZM((1.0, 2.0, 4.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 4.0, 4.0), (1.0, 2.0, 3.0, 4.0))', gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', gbox 'GBOX ZM((1.0, 2.0, 3.0, 5.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 3.0, 5.0), (1.0, 2.0, 3.0, 4.0))', gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (2.0, 2.0, 3.0, 4.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (2.0, 2.0, 3.0, 4.0))', gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 3.0, 3.0, 4.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 3.0, 3.0, 4.0))', gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 4.0, 4.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 4.0, 4.0))', gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 5.0))'); 
SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 5.0))', gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 

SELECT gbox_cmp(gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))', gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))'); 

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_gbox t1, tbl_gbox t2 where t1.b = t2.b;
SELECT count(*) FROM tbl_gbox t1, tbl_gbox t2 where t1.b <> t2.b;
SELECT count(*) FROM tbl_gbox t1, tbl_gbox t2 where t1.b < t2.b;
SELECT count(*) FROM tbl_gbox t1, tbl_gbox t2 where t1.b <= t2.b;
SELECT count(*) FROM tbl_gbox t1, tbl_gbox t2 where t1.b > t2.b;
SELECT count(*) FROM tbl_gbox t1, tbl_gbox t2 where t1.b >= t2.b;

SELECT count(*) FROM tbl_tgeompoint WHERE temp::gbox IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint WHERE temp::gbox IS NOT NULL;

SELECT count(*) FROM tbl_gbox t1, tbl_gbox t2 where t1.b && t2.b;
SELECT count(*) FROM tbl_gbox t1, tbl_gbox t2 where t1.b @> t2.b;
SELECT count(*) FROM tbl_gbox t1, tbl_gbox t2 where t1.b <@ t2.b;
SELECT count(*) FROM tbl_gbox t1, tbl_gbox t2 where t1.b ~= t2.b;

/*****************************************************************************/