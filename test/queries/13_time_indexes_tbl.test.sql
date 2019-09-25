-------------------------------------------------------------------------------

ANALYZE tbl_timestampset_big;
ANALYZE tbl_period_big;
ANALYZE tbl_periodset_big;

DROP INDEX IF EXISTS tbl_timestampset_big_gist_idx;
DROP INDEX IF EXISTS tbl_timestampset_big_spgist_idx;

DROP INDEX IF EXISTS tbl_period_big_gist_idx;
DROP INDEX IF EXISTS tbl_period_big_spgist_idx;

DROP INDEX IF EXISTS tbl_periodset_big_gist_idx;
DROP INDEX IF EXISTS tbl_periodset_big_spgist_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_timestampset_big_gist_idx ON tbl_timestampset_big USING GIST(ts);
CREATE INDEX tbl_period_big_gist_idx ON tbl_period_big USING GIST(p);
CREATE INDEX tbl_periodset_big_gist_idx ON tbl_periodset_big USING GIST(ps);

SELECT count(*) FROM tbl_timestampset_big WHERE ts && period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts @> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts <@ period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts #&> period '[2001-01-01, 2001-02-01]';

SELECT count(*) FROM tbl_period_big WHERE p && timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p @> timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p <@ timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p <<# timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p &<# timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p #>> timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p #&> timestamptz '2001-01-01';

SELECT count(*) FROM tbl_period_big WHERE p && timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p @> timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p <@ timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p <<# timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p &<# timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p #>> timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p #&> timestampset '{2001-01-01, 2001-02-01}';

SELECT count(*) FROM tbl_period_big WHERE p && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period_big WHERE p @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period_big WHERE p <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period_big WHERE p <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_period_big WHERE p &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_period_big WHERE p #>> period '[2001-11-01, 2001-12-01]';
SELECT count(*) FROM tbl_period_big WHERE p #&> period '[2001-11-01, 2001-12-01]';

SELECT count(*) FROM tbl_period_big WHERE p && periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p @> periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p <@ periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p <<# periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p &<# periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p #>> periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p #&> periodset '{[2001-01-01, 2001-02-01]}';

SELECT count(*) FROM tbl_periodset_big WHERE ps && period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps @> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps <@ period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps #&> period '[2001-01-01, 2001-02-01]';

DROP INDEX IF EXISTS tbl_timestampset_big_gist_idx;
DROP INDEX IF EXISTS tbl_period_big_gist_idx;
DROP INDEX IF EXISTS tbl_periodset_big_gist_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_timestampset_big_spgist_idx ON tbl_timestampset_big USING SPGIST(ts);
CREATE INDEX tbl_period_big_spgist_idx ON tbl_period_big USING SPGIST(p);
CREATE INDEX tbl_periodset_big_spgist_idx ON tbl_periodset_big USING SPGIST(ps);

SELECT count(*) FROM tbl_timestampset_big WHERE ts && period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts @> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts <@ period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts #&> period '[2001-01-01, 2001-02-01]';

SELECT count(*) FROM tbl_period_big WHERE p && timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p @> timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p <@ timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p <<# timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p &<# timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p #>> timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p #&> timestamptz '2001-01-01';

SELECT count(*) FROM tbl_period_big WHERE p && timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p @> timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p <@ timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p <<# timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p &<# timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p #>> timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p #&> timestampset '{2001-01-01, 2001-02-01}';

SELECT count(*) FROM tbl_period_big WHERE p && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period_big WHERE p @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period_big WHERE p <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period_big WHERE p <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_period_big WHERE p &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_period_big WHERE p #>> period '[2001-11-01, 2001-12-01]';
SELECT count(*) FROM tbl_period_big WHERE p #&> period '[2001-11-01, 2001-12-01]';

SELECT count(*) FROM tbl_period_big WHERE p && periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p @> periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p <@ periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p <<# periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p &<# periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p #>> periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p #&> periodset '{[2001-01-01, 2001-02-01]}';

SELECT count(*) FROM tbl_periodset_big WHERE ps && period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps @> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps <@ period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps #&> period '[2001-01-01, 2001-02-01]';

DROP INDEX IF EXISTS tbl_timestampset_big_spgist_idx;
DROP INDEX IF EXISTS tbl_period_big_spgist_idx;
DROP INDEX IF EXISTS tbl_periodset_big_spgist_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_period_test;
CREATE TABLE tbl_period_test AS
SELECT period '[2000-01-01,2000-01-02]';
VACUUM ANALYZE tbl_period_test;
DELETE FROM tbl_period_test;
INSERT INTO tbl_period_test
SELECT NULL::period UNION SELECT NULL::period;
VACUUM ANALYZE tbl_period_test;
DROP TABLE tbl_period_test;

-------------------------------------------------------------------------------