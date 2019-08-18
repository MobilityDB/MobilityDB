/*****************************************************************************
 * Period
 *****************************************************************************/

SELECT count(*) FROM tbl_tbool WHERE temp::period IS NOT NULL;
SELECT count(*) FROM tbl_ttext WHERE temp::period IS NOT NULL;

/*****************************************************************************
 * Tbox
 *****************************************************************************/

SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b = t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b <> t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b < t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b <= t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b > t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b >= t2.b;

SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b && t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b @> t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b <@ t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b ~= t2.b;

SELECT count(*) FROM tbl_int WHERE i::tbox IS NOT NULL;
SELECT count(*) FROM tbl_float WHERE f::tbox IS NOT NULL;
SELECT count(*) FROM tbl_intrange WHERE i::tbox IS NOT NULL;
SELECT count(*) FROM tbl_floatrange WHERE f::tbox IS NOT NULL;
SELECT count(*) FROM tbl_timestamptz WHERE t::tbox IS NOT NULL;
SELECT count(*) FROM tbl_period WHERE p::tbox IS NOT NULL;
SELECT count(*) FROM tbl_timestampset WHERE ts::tbox IS NOT NULL;
SELECT count(*) FROM tbl_periodset WHERE ps::tbox IS NOT NULL;
SELECT count(*) FROM tbl_tint WHERE temp::tbox IS NOT NULL;
SELECT count(*) FROM tbl_tfloat WHERE temp::tbox IS NOT NULL;

SELECT count(*) FROM tbl_int, tbl_timestamptz WHERE tbox(i, t) IS NOT NULL;
SELECT count(*) FROM tbl_intrange, tbl_timestamptz WHERE tbox(i, t) IS NOT NULL;
SELECT count(*) FROM tbl_float, tbl_timestamptz WHERE tbox(f, t) IS NOT NULL;
SELECT count(*) FROM tbl_floatrange, tbl_timestamptz WHERE tbox(f, t) IS NOT NULL;
SELECT count(*) FROM tbl_int, tbl_period WHERE tbox(i, p) IS NOT NULL;
SELECT count(*) FROM tbl_intrange, tbl_period WHERE tbox(i, p) IS NOT NULL;
SELECT count(*) FROM tbl_float, tbl_period WHERE tbox(f, p) IS NOT NULL;
SELECT count(*) FROM tbl_floatrange, tbl_period WHERE tbox(f, p) IS NOT NULL;

/*****************************************************************************/