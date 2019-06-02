/*****************************************************************************
 * Tbox
 *****************************************************************************/

SELECT tbox 'STBOX((1.0, 2.0), (1.0, 2.0))';

/* Errors */
SELECT tbox '(1, 2)';
SELECT tbox '((AA, 2))';
SELECT tbox '((1, AA))';
SELECT tbox '((1, 2, 3))';
SELECT tbox '((1, 2),(AA, 4))';
SELECT tbox '((1, 2),(3, AA))';
SELECT tbox '((1, 2),(3, 4)';

SELECT tbox(1,2,3,4);
SELECT tbox(4,3,2,1);

SELECT tbox_cmp(tbox '((1.0, 2.0), (1.0, 2.0))', tbox '((2.0, 2.0), (1.0, 2.0))'); 
SELECT tbox_cmp(tbox '((2.0, 2.0), (1.0, 2.0))', tbox '((1.0, 2.0), (1.0, 2.0))'); 
SELECT tbox_cmp(tbox '((1.0, 2.0), (1.0, 2.0))', tbox '((1.0, 3.0), (1.0, 2.0))'); 
SELECT tbox_cmp(tbox '((1.0, 3.0), (1.0, 2.0))', tbox '((1.0, 2.0), (1.0, 2.0))'); 
SELECT tbox_cmp(tbox '((1.0, 2.0), (1.0, 2.0))', tbox '((1.0, 2.0), (2.0, 2.0))'); 
SELECT tbox_cmp(tbox '((1.0, 2.0), (2.0, 2.0))', tbox '((1.0, 2.0), (1.0, 2.0))'); 
SELECT tbox_cmp(tbox '((1.0, 2.0), (1.0, 2.0))', tbox '((1.0, 2.0), (1.0, 3.0))'); 
SELECT tbox_cmp(tbox '((1.0, 2.0), (1.0, 3.0))', tbox '((1.0, 2.0), (1.0, 2.0))'); 

SELECT tbox_cmp(tbox '((1.0, 2.0), (1.0, 2.0))', tbox '((1.0, 2.0), (1.0, 2.0))'); 

SELECT tbox '((1.0, 2.0), (1.0, 2.0))' = floatrange '[1, 2]'::tbox; 

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b = t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b <> t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b < t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b <= t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b > t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b >= t2.b;

SELECT count(*) FROM tbl_tfloat WHERE temp::tbox IS NOT NULL;
SELECT count(*) FROM tbl_tfloat WHERE temp::tbox IS NOT NULL;

SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b && t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b @> t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b <@ t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b ~= t2.b;

/*****************************************************************************/