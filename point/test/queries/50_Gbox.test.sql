/*****************************************************************************
 * Gbox
 *****************************************************************************/

SELECT gbox 'GBOX((1.0, 2.0), (1.0, 2.0))';
SELECT gbox 'GBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT gbox 'GBOX M((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT gbox 'GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))';
SELECT gbox 'GEODBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

SELECT gbox(1,2,3,4);
SELECT gbox(1,2,3,4,5,6);
SELECT gbox(1,2,3,4,5,6,7,8);
SELECT gbox3dm(1,2,3,4,5,6);
SELECT geodbox(1,2,3,4,5,6);

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_gbox t1, tbl_gbox t2 where t1.b && t2.b;
SELECT count(*) FROM tbl_gbox t1, tbl_gbox t2 where t1.b @> t2.b;
SELECT count(*) FROM tbl_gbox t1, tbl_gbox t2 where t1.b <@ t2.b;
SELECT count(*) FROM tbl_gbox t1, tbl_gbox t2 where t1.b ~= t2.b;

/*****************************************************************************/