-------------------------------------------------------------------------------
-- Tests for extensions of range data type.
-- File Range.c
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_intrange t1, tbl_int t2 WHERE t1.i << t2.i;
SELECT count(*) FROM tbl_int t1, tbl_intrange t2 WHERE t1.i << t2.i;

SELECT count(*) FROM tbl_intrange t1, tbl_int t2 WHERE t1.i >> t2.i;
SELECT count(*) FROM tbl_int t1, tbl_intrange t2 WHERE t1.i >> t2.i;

SELECT count(*) FROM tbl_intrange t1, tbl_int t2 WHERE t1.i &< t2.i;
SELECT count(*) FROM tbl_int t1, tbl_intrange t2 WHERE t1.i &< t2.i;

SELECT count(*) FROM tbl_intrange t1, tbl_int t2 WHERE t1.i &> t2.i;
SELECT count(*) FROM tbl_int t1, tbl_intrange t2 WHERE t1.i &> t2.i;

SELECT count(*) FROM tbl_intrange t1, tbl_int t2 WHERE t1.i -|- t2.i;
SELECT count(*) FROM tbl_int t1, tbl_intrange t2 WHERE t1.i -|- t2.i;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_floatrange t1, tbl_float t2 WHERE t1.f << t2.f;
SELECT count(*) FROM tbl_float t1, tbl_floatrange t2 WHERE t1.f << t2.f;

SELECT count(*) FROM tbl_floatrange t1, tbl_float t2 WHERE t1.f >> t2.f;
SELECT count(*) FROM tbl_float t1, tbl_floatrange t2 WHERE t1.f >> t2.f;

SELECT count(*) FROM tbl_floatrange t1, tbl_float t2 WHERE t1.f &< t2.f;
SELECT count(*) FROM tbl_float t1, tbl_floatrange t2 WHERE t1.f &< t2.f;

SELECT count(*) FROM tbl_floatrange t1, tbl_float t2 WHERE t1.f &> t2.f;
SELECT count(*) FROM tbl_float t1, tbl_floatrange t2 WHERE t1.f &> t2.f;

SELECT count(*) FROM tbl_floatrange t1, tbl_float t2 WHERE t1.f -|- t2.f;
SELECT count(*) FROM tbl_float t1, tbl_floatrange t2 WHERE t1.f -|- t2.f;

-------------------------------------------------------------------------------
