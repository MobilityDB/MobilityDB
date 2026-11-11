-- Smoke tests for the |=| nearest-approach-distance operator and KNN
-- ordering via GiST.

-------------------------------------------------------------------------------
-- Self-distance is zero (every value is at distance 0 from its own bbox).
-------------------------------------------------------------------------------

-- This test creates its indexes on copies of the tables of the setup scripts,
-- which hide those tables in its session: every test reads them, and an
-- index on them would change the plans of the other tests.
CREATE TEMP TABLE tbl_tpcpoint AS TABLE tbl_tpcpoint;
ANALYZE tbl_tpcpoint;

SELECT bool_and((temp |=| temp) = 0) FROM tbl_tpcpoint;
SELECT bool_and((temp |=| temp) = 0) FROM tbl_tpcpatch;
SELECT bool_and((b |=| b) = 0) FROM tbl_tpcbox;

-------------------------------------------------------------------------------
-- KNN ordering — index scan should return the top-N rows in the same
-- order as a sequential scan.
-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_gist_dist_idx ON tbl_tpcpoint USING gist(temp);

SET enable_seqscan = on;
SET enable_indexscan = off;
SET enable_bitmapscan = off;
SELECT k, ((temp |=| tpcboxZT(0, 0, 0, 0, 0, 0,
  tstzspan '[2001-06-01, 2001-06-30]', 1)) > 0)::text AS finite
FROM tbl_tpcpoint
ORDER BY temp |=| tpcboxZT(0, 0, 0, 0, 0, 0,
  tstzspan '[2001-06-01, 2001-06-30]', 1)
LIMIT 5;

SET enable_seqscan = off;
SET enable_indexscan = on;
SET enable_bitmapscan = on;
SELECT k, ((temp |=| tpcboxZT(0, 0, 0, 0, 0, 0,
  tstzspan '[2001-06-01, 2001-06-30]', 1)) > 0)::text AS finite
FROM tbl_tpcpoint
ORDER BY temp |=| tpcboxZT(0, 0, 0, 0, 0, 0,
  tstzspan '[2001-06-01, 2001-06-30]', 1)
LIMIT 5;

DROP INDEX tbl_tpcpoint_gist_dist_idx;
RESET enable_seqscan;
RESET enable_indexscan;
RESET enable_bitmapscan;

-------------------------------------------------------------------------------
