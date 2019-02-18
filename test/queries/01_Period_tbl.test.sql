-------------------------------------------------------------------------------
-- Tests for period data type.
-- File Period.c
-------------------------------------------------------------------------------

select tstzrange(p) from tbl_period;
select period(r) from tbl_tstzrange;

select lower(p) from tbl_period;
select upper(p) from tbl_period;
select lower_inc(p) from tbl_period;
select upper_inc(p) from tbl_period;
select duration(p) from tbl_period;
select shift(p, '5 min') from tbl_period;

select count(*) from tbl_period t1, tbl_period t2 where period_cmp(t1.p, t2.p) = -1;
select count(*) from tbl_period t1, tbl_period t2 where t1.p < t2.p;
select count(*) from tbl_period t1, tbl_period t2 where t1.p <= t2.p;
select count(*) from tbl_period t1, tbl_period t2 where t1.p > t2.p;
select count(*) from tbl_period t1, tbl_period t2 where t1.p >= t2.p;

-------------------------------------------------------------------------------
