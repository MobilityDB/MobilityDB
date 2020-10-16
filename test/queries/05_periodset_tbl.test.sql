﻿-------------------------------------------------------------------------------
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

SELECT memSize(ps) FROM tbl_periodset;
SELECT period(ps) FROM tbl_periodset;
SELECT timespan(ps) FROM tbl_periodset;

SELECT numPeriods(ps) FROM tbl_periodset;
SELECT startPeriod(ps) FROM tbl_periodset;
SELECT endPeriod(ps) FROM tbl_periodset;
SELECT periodN(ps, 1) FROM tbl_periodset;
SELECT periods(ps) FROM tbl_periodset;

SELECT numTimestamps(ps) FROM tbl_periodset;
SELECT startTimestamp(ps) FROM tbl_periodset;
SELECT endTimestamp(ps) FROM tbl_periodset;
SELECT timestampN(ps, 0) FROM tbl_periodset;
SELECT timestamps(ps) FROM tbl_periodset;

SELECT shift(ps, '5 min') FROM tbl_periodset;

SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE periodset_cmp(t1.ps, t2.ps) = -1;
SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps = t2.ps;
SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <> t2.ps;
SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps < t2.ps;
SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <= t2.ps;
SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps > t2.ps;
SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps >= t2.ps;

-------------------------------------------------------------------------------
