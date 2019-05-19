-------------------------------------------------------------------------------
-- Tests for timestampset data type.
-- File TimestampSet.c
-------------------------------------------------------------------------------

-- Send/receive functions

COPY tbl_timestampset TO '/tmp/tbl_timestampset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_timestampset_tmp;
CREATE TABLE tbl_timestampset_tmp AS TABLE tbl_timestampset WITH NO DATA;
COPY tbl_timestampset_tmp FROM '/tmp/tbl_timestampset' (FORMAT BINARY);
DROP TABLE tbl_timestampset_tmp;

-------------------------------------------------------------------------------
-- Constructor

SELECT timestampset(array_agg(DISTINCT t ORDER BY t)) FROM tbl_timestamptz WHERE t IS NOT NULL LIMIT 10;

-------------------------------------------------------------------------------
-- Cast

SELECT count(*) FROM tbl_timestamptz WHERE t::timestampset IS NOT NULL;

-------------------------------------------------------------------------------
-- Functions

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
