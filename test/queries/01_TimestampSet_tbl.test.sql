-------------------------------------------------------------------------------
-- Tests for timestamp set data type.
-- File TimestampSet.c
-------------------------------------------------------------------------------

select size(ts) from tbl_timestampset;
select timespan(ts) from tbl_timestampset;

select numTimestamps(ts) from tbl_timestampset;
select startTimestamp(ts) from tbl_timestampset;
select endTimestamp(ts) from tbl_timestampset;
select timestampN(ts, 0) from tbl_timestampset;
select timestamps(ts) from tbl_timestampset;

select shift(ts, '5 min') from tbl_timestampset;

select count(*) from tbl_timestampset t1, tbl_timestampset t2 where timestampset_cmp(t1.ts, t2.ts) = -1;
select count(*) from tbl_timestampset t1, tbl_timestampset t2 where t1.ts = t2.ts;
select count(*) from tbl_timestampset t1, tbl_timestampset t2 where t1.ts <> t2.ts;
select count(*) from tbl_timestampset t1, tbl_timestampset t2 where t1.ts < t2.ts;
select count(*) from tbl_timestampset t1, tbl_timestampset t2 where t1.ts <= t2.ts;
select count(*) from tbl_timestampset t1, tbl_timestampset t2 where t1.ts > t2.ts;
select count(*) from tbl_timestampset t1, tbl_timestampset t2 where t1.ts >= t2.ts;

-------------------------------------------------------------------------------
