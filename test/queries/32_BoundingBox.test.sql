/*****************************************************************************
 * Period
 *****************************************************************************/

SELECT count(*) FROM tbl_tbool WHERE temp::period IS NOT NULL;
SELECT count(*) FROM tbl_ttext WHERE temp::period IS NOT NULL;

/*****************************************************************************
 * Box
 *****************************************************************************/

SELECT count(*) FROM tbl_box t1, tbl_box t2 where t1.b = t2.b;
SELECT count(*) FROM tbl_box t1, tbl_box t2 where t1.b <> t2.b;
SELECT count(*) FROM tbl_box t1, tbl_box t2 where t1.b < t2.b;
SELECT count(*) FROM tbl_box t1, tbl_box t2 where t1.b <= t2.b;
SELECT count(*) FROM tbl_box t1, tbl_box t2 where t1.b > t2.b;
SELECT count(*) FROM tbl_box t1, tbl_box t2 where t1.b >= t2.b;

SELECT count(*) FROM tbl_box t1, tbl_box t2 where t1.b && t2.b;
SELECT count(*) FROM tbl_box t1, tbl_box t2 where t1.b @> t2.b;
SELECT count(*) FROM tbl_box t1, tbl_box t2 where t1.b <@ t2.b;
SELECT count(*) FROM tbl_box t1, tbl_box t2 where t1.b ~= t2.b;

SELECT count(*) FROM tbl_int WHERE i::box IS NOT NULL;
SELECT count(*) FROM tbl_float WHERE f::box IS NOT NULL;
SELECT count(*) FROM tbl_intrange WHERE i::box IS NOT NULL;
SELECT count(*) FROM tbl_floatrange WHERE f::box IS NOT NULL;
SELECT count(*) FROM tbl_timestamptz WHERE t::box IS NOT NULL;
SELECT count(*) FROM tbl_period WHERE p::box IS NOT NULL;
SELECT count(*) FROM tbl_timestampset WHERE ts::box IS NOT NULL;
SELECT count(*) FROM tbl_periodset WHERE ps::box IS NOT NULL;
SELECT count(*) FROM tbl_tint WHERE temp::box IS NOT NULL;
SELECT count(*) FROM tbl_tfloat WHERE temp::box IS NOT NULL;

SELECT count(*) FROM tbl_int, tbl_timestamptz WHERE box(i, t) IS NOT NULL;
SELECT count(*) FROM tbl_intrange, tbl_timestamptz WHERE box(i, t) IS NOT NULL;
SELECT count(*) FROM tbl_float, tbl_timestamptz WHERE box(f, t) IS NOT NULL;
SELECT count(*) FROM tbl_floatrange, tbl_timestamptz WHERE box(f, t) IS NOT NULL;
SELECT count(*) FROM tbl_int, tbl_period WHERE box(i, p) IS NOT NULL;
SELECT count(*) FROM tbl_intrange, tbl_period WHERE box(i, p) IS NOT NULL;
SELECT count(*) FROM tbl_float, tbl_period WHERE box(f, p) IS NOT NULL;
SELECT count(*) FROM tbl_floatrange, tbl_period WHERE box(f, p) IS NOT NULL;

/*****************************************************************************/