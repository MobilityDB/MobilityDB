-- SP-GiST index test: build quadtree + kdtree indexes, run a few
-- box-overlap queries and verify the answers match a sequential-scan
-- plan.

-------------------------------------------------------------------------------
-- tpcpoint quadtree
-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_quadtree_idx ON tbl_tpcpoint USING spgist(temp);

SET enable_seqscan = on;
SET enable_indexscan = off;
SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_count FROM tbl_tpcpoint
WHERE temp && tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1);

SET enable_seqscan = off;
SET enable_indexscan = on;
SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_count FROM tbl_tpcpoint
WHERE temp && tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1);

DROP INDEX tbl_tpcpoint_quadtree_idx;

-------------------------------------------------------------------------------
-- tpcpoint kdtree
-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_kdtree_idx ON tbl_tpcpoint
  USING spgist(temp tpcpoint_kdtree_ops);

SELECT COUNT(*) AS kdtree_count FROM tbl_tpcpoint
WHERE temp && tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1);

DROP INDEX tbl_tpcpoint_kdtree_idx;

-------------------------------------------------------------------------------
-- tpcpatch quadtree + kdtree
-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_quadtree_idx ON tbl_tpcpatch USING spgist(temp);

SET enable_seqscan = on;
SET enable_indexscan = off;
SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_count FROM tbl_tpcpatch
WHERE temp && tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1);

SET enable_seqscan = off;
SET enable_indexscan = on;
SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_count FROM tbl_tpcpatch
WHERE temp && tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1);

DROP INDEX tbl_tpcpatch_quadtree_idx;

CREATE INDEX tbl_tpcpatch_kdtree_idx ON tbl_tpcpatch
  USING spgist(temp tpcpatch_kdtree_ops);

SELECT COUNT(*) AS kdtree_count FROM tbl_tpcpatch
WHERE temp && tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1);

DROP INDEX tbl_tpcpatch_kdtree_idx;

RESET enable_seqscan;
RESET enable_indexscan;
RESET enable_bitmapscan;

-------------------------------------------------------------------------------
-- Nearest approach ordering answered by the space-partitioning indexes
-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_knn_quadtree_idx ON tbl_tpcpoint USING spgist(temp);
WITH test AS (
  SELECT temp |=| tpcboxZT(200, 200, 200, 210, 210, 210,
  tstzspan '[2001-06-01, 2001-12-31]', 1) AS distance
  FROM tbl_tpcpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
DROP INDEX tbl_tpcpoint_knn_quadtree_idx;

CREATE INDEX tbl_tpcpoint_knn_kdtree_idx ON tbl_tpcpoint
  USING spgist(temp tpcpoint_kdtree_ops);
WITH test AS (
  SELECT temp |=| tpcboxZT(200, 200, 200, 210, 210, 210,
  tstzspan '[2001-06-01, 2001-12-31]', 1) AS distance
  FROM tbl_tpcpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
DROP INDEX tbl_tpcpoint_knn_kdtree_idx;

CREATE INDEX tbl_tpcpatch_knn_quadtree_idx ON tbl_tpcpatch USING spgist(temp);
WITH test AS (
  SELECT temp |=| tpcboxXT(200, 200, 210, 210,
  tstzspan '[2001-06-01, 2001-12-31]', 1) AS distance
  FROM tbl_tpcpatch ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
DROP INDEX tbl_tpcpatch_knn_quadtree_idx;

CREATE INDEX tbl_tpcpatch_knn_kdtree_idx ON tbl_tpcpatch
  USING spgist(temp tpcpatch_kdtree_ops);
WITH test AS (
  SELECT temp |=| tpcboxXT(200, 200, 210, 210,
  tstzspan '[2001-06-01, 2001-12-31]', 1) AS distance
  FROM tbl_tpcpatch ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
DROP INDEX tbl_tpcpatch_knn_kdtree_idx;

-- The scan with no index answers as the two indexes do
WITH test AS (
  SELECT temp |=| tpcboxXT(200, 200, 210, 210,
  tstzspan '[2001-06-01, 2001-12-31]', 1) AS distance
  FROM tbl_tpcpatch ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

-------------------------------------------------------------------------------
-- Coverage of all the same and order by logic in SP-GiST indexes

CREATE TABLE tbl_tpcpoint_big_allthesame AS
  SELECT k, tpcpoint(PC_MakePoint(1, ARRAY[5.0, 5.0, 5.0]::float[]), timeSpan(temp)) AS temp
  FROM tbl_tpcpoint WHERE temp IS NOT NULL;
CREATE INDEX tbl_tpcpoint_big_allthesame_quadtree_idx ON tbl_tpcpoint_big_allthesame USING spgist(temp);
ANALYZE tbl_tpcpoint_big_allthesame;

-- EXPLAIN ANALYZE

DROP TABLE tbl_tpcpoint_big_allthesame;

-------------------------------------------------------------------------------
-- An instant as the query value, given as a constant, which the planner
-- estimates, and through a subquery, which only the index reads

CREATE INDEX tbl_tpcpoint_quadtree_idx ON tbl_tpcpoint USING spgist(temp);
CREATE INDEX tbl_tpcpatch_quadtree_idx ON tbl_tpcpatch USING spgist(temp);

SET enable_seqscan = on; SET enable_indexscan = off; SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_inst FROM tbl_tpcpoint
WHERE temp && tpcpoint(pcpoint(1, 3.09, 47.75, 20.5),
  timestamptz '2001-09-10 22:14:00-07');
SELECT COUNT(*) AS seq_self FROM tbl_tpcpoint t1
WHERE t1.temp && (SELECT temp FROM tbl_tpcpoint WHERE temp IS NOT NULL ORDER BY k LIMIT 1);
SELECT COUNT(*) AS seq_self_patch FROM tbl_tpcpatch t1
WHERE t1.temp && (SELECT temp FROM tbl_tpcpatch WHERE temp IS NOT NULL ORDER BY k LIMIT 1);

SET enable_seqscan = off; SET enable_indexscan = on; SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_inst FROM tbl_tpcpoint
WHERE temp && tpcpoint(pcpoint(1, 3.09, 47.75, 20.5),
  timestamptz '2001-09-10 22:14:00-07');
SELECT COUNT(*) AS idx_self FROM tbl_tpcpoint t1
WHERE t1.temp && (SELECT temp FROM tbl_tpcpoint WHERE temp IS NOT NULL ORDER BY k LIMIT 1);
SELECT COUNT(*) AS idx_self_patch FROM tbl_tpcpatch t1
WHERE t1.temp && (SELECT temp FROM tbl_tpcpatch WHERE temp IS NOT NULL ORDER BY k LIMIT 1);
RESET enable_seqscan; RESET enable_indexscan; RESET enable_bitmapscan;

DROP INDEX tbl_tpcpoint_quadtree_idx;
DROP INDEX tbl_tpcpatch_quadtree_idx;

-------------------------------------------------------------------------------
