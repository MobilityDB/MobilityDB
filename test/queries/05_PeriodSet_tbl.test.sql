-------------------------------------------------------------------------------
-- Tests for period set data type.
-- File PeriodSet.c
--------------------------------------------------------------------------------
-- Send/receive functions

COPY tbl_periodset TO '/tmp/tbl_periodset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_periodset_tmp;
CREATE TABLE tbl_periodset_tmp AS TABLE tbl_periodset WITH NO DATA;
COPY tbl_periodset_tmp FROM '/tmp/tbl_periodset' (FORMAT BINARY);
DROP TABLE tbl_periodset_tmp;

-------------------------------------------------------------------------------

select memSize(ps) from tbl_periodset;
select timespan(ps) from tbl_periodset;
select duration(ps) from tbl_periodset;

select numPeriods(ps) from tbl_periodset;
select startPeriod(ps) from tbl_periodset;
select endPeriod(ps) from tbl_periodset;
select periodN(ps, 1) from tbl_periodset;
select periods(ps) from tbl_periodset;

select numTimestamps(ps) from tbl_periodset;
select startTimestamp(ps) from tbl_periodset;
select endTimestamp(ps) from tbl_periodset;
select timestampN(ps, 0) from tbl_periodset;
select timestamps(ps) from tbl_periodset;

select shift(ps, '5 min') from tbl_periodset;

select count(*) from tbl_periodset t1, tbl_periodset t2 where periodset_cmp(t1.ps, t2.ps) = -1;
select count(*) from tbl_periodset t1, tbl_periodset t2 where t1.ps = t2.ps;
select count(*) from tbl_periodset t1, tbl_periodset t2 where t1.ps <> t2.ps;
select count(*) from tbl_periodset t1, tbl_periodset t2 where t1.ps < t2.ps;
select count(*) from tbl_periodset t1, tbl_periodset t2 where t1.ps <= t2.ps;
select count(*) from tbl_periodset t1, tbl_periodset t2 where t1.ps > t2.ps;
select count(*) from tbl_periodset t1, tbl_periodset t2 where t1.ps >= t2.ps;

-------------------------------------------------------------------------------
