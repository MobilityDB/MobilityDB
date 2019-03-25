-------------------------------------------------------------------------------
-- Tests for extensions of range data type.
-- File Range.c
-------------------------------------------------------------------------------

select count(*) from tbl_intrange t1, tbl_int t2 where t1.i << t2.i;
select count(*) from tbl_int t1, tbl_intrange t2 where t1.i << t2.i;

select count(*) from tbl_intrange t1, tbl_int t2 where t1.i >> t2.i;
select count(*) from tbl_int t1, tbl_intrange t2 where t1.i >> t2.i;

select count(*) from tbl_intrange t1, tbl_int t2 where t1.i &< t2.i;
select count(*) from tbl_int t1, tbl_intrange t2 where t1.i &< t2.i;

select count(*) from tbl_intrange t1, tbl_int t2 where t1.i &> t2.i;
select count(*) from tbl_int t1, tbl_intrange t2 where t1.i &> t2.i;

select count(*) from tbl_intrange t1, tbl_int t2 where t1.i -|- t2.i;
select count(*) from tbl_int t1, tbl_intrange t2 where t1.i -|- t2.i;

-------------------------------------------------------------------------------

select count(*) from tbl_floatrange t1, tbl_float t2 where t1.f << t2.f;
select count(*) from tbl_float t1, tbl_floatrange t2 where t1.f << t2.f;

select count(*) from tbl_floatrange t1, tbl_float t2 where t1.f >> t2.f;
select count(*) from tbl_float t1, tbl_floatrange t2 where t1.f >> t2.f;

select count(*) from tbl_floatrange t1, tbl_float t2 where t1.f &< t2.f;
select count(*) from tbl_float t1, tbl_floatrange t2 where t1.f &< t2.f;

select count(*) from tbl_floatrange t1, tbl_float t2 where t1.f &> t2.f;
select count(*) from tbl_float t1, tbl_floatrange t2 where t1.f &> t2.f;

select count(*) from tbl_floatrange t1, tbl_float t2 where t1.f -|- t2.f;
select count(*) from tbl_float t1, tbl_floatrange t2 where t1.f -|- t2.f;

-------------------------------------------------------------------------------
