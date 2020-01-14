-------------------------------------------------------------------------------
-- Tests for period data type.
-- File Period.c
--------------------------------------------------------------------------------
-- Send/receive functions

COPY tbl_period TO '/tmp/tbl_period' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_period_tmp;
CREATE TABLE tbl_period_tmp AS TABLE tbl_period WITH NO DATA;
COPY tbl_period_tmp FROM '/tmp/tbl_period' (FORMAT BINARY);
DROP TABLE tbl_period_tmp;

-------------------------------------------------------------------------------

SELECT max(timespan(period(t, t + i))) FROM tbl_timestamptz, tbl_interval;
SELECT max(timespan(period(t, t + i, true, true))) FROM tbl_timestamptz, tbl_interval;
SELECT max(timespan(period(t, t + i, true, false))) FROM tbl_timestamptz, tbl_interval;
SELECT max(timespan(period(t, t + i, false, true))) FROM tbl_timestamptz, tbl_interval;
SELECT max(timespan(period(t, t + i, false, false))) FROM tbl_timestamptz, tbl_interval;

SELECT tstzrange(p) FROM tbl_period;
SELECT period(r) FROM tbl_tstzrange;
SELECT t::period FROM tbl_timestamptz;

SELECT lower(p) FROM tbl_period;
SELECT upper(p) FROM tbl_period;
SELECT lower_inc(p) FROM tbl_period;
SELECT upper_inc(p) FROM tbl_period;
SELECT timespan(p) FROM tbl_period;
SELECT shift(p, '5 min') FROM tbl_period;

SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE period_cmp(t1.p, t2.p) = -1;
SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p < t2.p;
SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <= t2.p;
SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p > t2.p;
SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p >= t2.p;

SELECT max(period_hash(p)) FROM tbl_period;
SELECT max(period_hash_extended(p)) FROM tbl_period;

-------------------------------------------------------------------------------
