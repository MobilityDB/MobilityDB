-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp >= t2.temp;

-- The order is total: every pair falls on exactly one side
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2
WHERE (t1.temp < t2.temp)::int + (t1.temp = t2.temp)::int + (t1.temp > t2.temp)::int <> 1;

-------------------------------------------------------------------------------
-- Ever and always comparison against a value
-------------------------------------------------------------------------------

-- A chain of another length is simply not equal, so the whole cross join
-- answers rather than refusing the pairs whose counts differ
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_posechain t2 WHERE t1.temp ?= t2.pc;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_posechain t2 WHERE t1.temp %= t2.pc;

-- The join does span differing counts, so the answers above are false rather
-- than absent
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_posechain t2
WHERE numPoses(t2.pc) <> numPoses(t1.temp);

-------------------------------------------------------------------------------
