/*****************************************************************************
 * Box
 *****************************************************************************/

SELECT count(*) FROM tbl_box t1, tbl_box t2 where t1.b && t2.b;
SELECT count(*) FROM tbl_box t1, tbl_box t2 where t1.b @> t2.b;
SELECT count(*) FROM tbl_box t1, tbl_box t2 where t1.b <@ t2.b;
SELECT count(*) FROM tbl_box t1, tbl_box t2 where t1.b ~= t2.b;

/*****************************************************************************/