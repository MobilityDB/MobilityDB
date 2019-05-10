-------------------------------------------------------------------------------
-- Tests for timestamp set data type.
-- File TimestampSet.c
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Constructor
-------------------------------------------------------------------------------

SELECT timestampset(ARRAY [timestamp '2000-01-01', '2000-01-02', '2000-01-03']);

-------------------------------------------------------------------------------
-- Cast
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_timestamptz WHERE t::timestampset IS NOT NULL;

-------------------------------------------------------------------------------
-- Functions
-------------------------------------------------------------------------------

SELECT memSize(ts) FROM tbl_timestampset;
SELECT timespan(ts) FROM tbl_timestampset;

SELECT numTimestamps(ts) FROM tbl_timestampset;
SELECT startTimestamp(ts) FROM tbl_timestampset;
SELECT endTimestamp(ts) FROM tbl_timestampset;
SELECT timestampN(ts, 0) FROM tbl_timestampset;
SELECT timestamps(ts) FROM tbl_timestampset;

SELECT shift(ts, '5 min') FROM tbl_timestampset;

SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE timestampset_cmp(t1.ts, t2.ts) = -1;
SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts = t2.ts;
SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <> t2.ts;
SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts < t2.ts;
SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <= t2.ts;
SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts > t2.ts;
SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts >= t2.ts;

-------------------------------------------------------------------------------
