-------------------------------------------------------------------------------
-- The multi-entry R-tree mgist, quad-tree and k-d tree mspgist indexes of a
-- tjsonb, keyed by the tstzspanes of the pieces of the value, one per split (equi,
-- seg, bin). Under each index the ever and always overlaps, with each right
-- argument they declare, answer the rows a sequential scan gives, from a plan that
-- probes the index.
-------------------------------------------------------------------------------

CREATE FUNCTION pg_temp.test_probes_index(query text)
RETURNS boolean LANGUAGE plpgsql AS $$
DECLARE
  line text;
BEGIN
  FOR line IN EXECUTE 'EXPLAIN (COSTS OFF) ' || query LOOP
    IF line LIKE '%Index Cond%' THEN
      RETURN true;
    END IF;
  END LOOP;
  RETURN false;
END;
$$;

DROP TABLE IF EXISTS tbl_tjsonb_idx;
CREATE TEMP TABLE tbl_tjsonb_idx AS
WITH base(k, temp) AS (
  SELECT k, temp FROM tbl_tjsonb_inst
  UNION ALL
  SELECT k + 100, temp FROM tbl_tjsonb_discseq
  UNION ALL
  SELECT k + 200, temp FROM tbl_tjsonb_contseq
  UNION ALL
  SELECT k + 300, temp FROM tbl_tjsonb_seqset )
SELECT k + n * 400 AS k, shiftTime(temp, n * interval '1 week') AS temp
FROM base, generate_series(0, 25) AS n;
ANALYZE tbl_tjsonb_idx;

-- The values and periods the operators are given. Those of the fixture are
-- random, so they meet few values exactly: the others come from the rows of the
-- table, so that every operator has rows to answer.
DROP TABLE IF EXISTS tbl_tjsonb_src;
CREATE TEMP TABLE tbl_tjsonb_src AS
SELECT k, temp, temp::tstzspan AS t
FROM tbl_tjsonb_idx WHERE k % 900 = 0;

-- A value of the fixture, a value of the table, and one whose period ends where
-- that of the value starts
DROP TABLE IF EXISTS tbl_tjsonb_val;
CREATE TEMP TABLE tbl_tjsonb_val AS
SELECT temp, false AS knn FROM tbl_tjsonb WHERE k % 20 = 0
UNION ALL
SELECT temp, true FROM tbl_tjsonb_src
UNION ALL
SELECT shiftTime(temp, lower(t) - upper(t)), false FROM tbl_tjsonb_src;
ANALYZE tbl_tjsonb_val;

-- A period of the fixture, the period of a value, one around it, one inside it,
-- and one that meets it
DROP TABLE IF EXISTS tbl_tjsonb_span;
CREATE TEMP TABLE tbl_tjsonb_span AS
SELECT t FROM tbl_tstzspan WHERE k % 20 = 0
UNION ALL
SELECT t FROM tbl_tjsonb_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) - interval '1 day', upper(t) + interval '1 day'))
FROM tbl_tjsonb_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2))
FROM tbl_tjsonb_src
UNION ALL
SELECT tstzspan(format('[%s, %s%s', lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END))
FROM tbl_tjsonb_src;
ANALYZE tbl_tjsonb_span;

DROP TABLE IF EXISTS test_tjsonb_mest;
CREATE TEMP TABLE test_tjsonb_mest(op text, no_idx text,
  rtree_equi_idx text, rtree_equi_probe boolean,
  rtree_seg_idx text, rtree_seg_probe boolean,
  rtree_bin_idx text, rtree_bin_probe boolean,
  quadtree_equi_idx text, quadtree_equi_probe boolean,
  quadtree_seg_idx text, quadtree_seg_probe boolean,
  quadtree_bin_idx text, quadtree_bin_probe boolean,
  kdtree_equi_idx text, kdtree_equi_probe boolean,
  kdtree_seg_idx text, kdtree_seg_probe boolean,
  kdtree_bin_idx text, kdtree_bin_probe boolean);

INSERT INTO test_tjsonb_mest(op, no_idx) VALUES ('?&& tstzspan', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t)::text);
INSERT INTO test_tjsonb_mest(op, no_idx) VALUES ('%&& tstzspan', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t)::text);

-------------------------------------------------------------------------------

CREATE INDEX tbl_tjsonb_idx_rtree_equi ON tbl_tjsonb_idx
  USING mgist(temp tjsonb_mrtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tjsonb_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t')
WHERE op = '?&& tstzspan';
UPDATE test_tjsonb_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t')
WHERE op = '%&& tstzspan';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tjsonb_idx_rtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tjsonb_idx_rtree_seg ON tbl_tjsonb_idx
  USING mgist(temp tjsonb_mrtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tjsonb_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t')
WHERE op = '?&& tstzspan';
UPDATE test_tjsonb_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t')
WHERE op = '%&& tstzspan';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tjsonb_idx_rtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tjsonb_idx_rtree_bin ON tbl_tjsonb_idx
  USING mgist(temp tjsonb_mrtree_binsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tjsonb_mest SET rtree_bin_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t)::text,
  rtree_bin_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t')
WHERE op = '?&& tstzspan';
UPDATE test_tjsonb_mest SET rtree_bin_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t)::text,
  rtree_bin_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t')
WHERE op = '%&& tstzspan';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tjsonb_idx_rtree_bin;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tjsonb_idx_quadtree_equi ON tbl_tjsonb_idx
  USING mspgist(temp tjsonb_mquadtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tjsonb_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t')
WHERE op = '?&& tstzspan';
UPDATE test_tjsonb_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t')
WHERE op = '%&& tstzspan';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tjsonb_idx_quadtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tjsonb_idx_quadtree_seg ON tbl_tjsonb_idx
  USING mspgist(temp tjsonb_mquadtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tjsonb_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t')
WHERE op = '?&& tstzspan';
UPDATE test_tjsonb_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t')
WHERE op = '%&& tstzspan';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tjsonb_idx_quadtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tjsonb_idx_quadtree_bin ON tbl_tjsonb_idx
  USING mspgist(temp tjsonb_mquadtree_binsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tjsonb_mest SET quadtree_bin_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t)::text,
  quadtree_bin_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t')
WHERE op = '?&& tstzspan';
UPDATE test_tjsonb_mest SET quadtree_bin_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t)::text,
  quadtree_bin_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t')
WHERE op = '%&& tstzspan';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tjsonb_idx_quadtree_bin;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tjsonb_idx_kdtree_equi ON tbl_tjsonb_idx
  USING mspgist(temp tjsonb_mkdtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tjsonb_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t')
WHERE op = '?&& tstzspan';
UPDATE test_tjsonb_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t')
WHERE op = '%&& tstzspan';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tjsonb_idx_kdtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tjsonb_idx_kdtree_seg ON tbl_tjsonb_idx
  USING mspgist(temp tjsonb_mkdtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tjsonb_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t')
WHERE op = '?&& tstzspan';
UPDATE test_tjsonb_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t')
WHERE op = '%&& tstzspan';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tjsonb_idx_kdtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tjsonb_idx_kdtree_bin ON tbl_tjsonb_idx
  USING mspgist(temp tjsonb_mkdtree_binsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tjsonb_mest SET kdtree_bin_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t)::text,
  kdtree_bin_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ?&& t2.t')
WHERE op = '?&& tstzspan';
UPDATE test_tjsonb_mest SET kdtree_bin_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t)::text,
  kdtree_bin_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp %&& t2.t')
WHERE op = '%&& tstzspan';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tjsonb_idx_kdtree_bin;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tjsonb_idx;
SELECT COUNT(*) FROM test_tjsonb_mest;
SELECT op, no_idx FROM test_tjsonb_mest ORDER BY op;
SELECT * FROM test_tjsonb_mest
WHERE no_idx IS DISTINCT FROM rtree_equi_idx OR NOT rtree_equi_probe OR
  no_idx IS DISTINCT FROM rtree_seg_idx OR NOT rtree_seg_probe OR
  no_idx IS DISTINCT FROM rtree_bin_idx OR NOT rtree_bin_probe OR
  no_idx IS DISTINCT FROM quadtree_equi_idx OR NOT quadtree_equi_probe OR
  no_idx IS DISTINCT FROM quadtree_seg_idx OR NOT quadtree_seg_probe OR
  no_idx IS DISTINCT FROM quadtree_bin_idx OR NOT quadtree_bin_probe OR
  no_idx IS DISTINCT FROM kdtree_equi_idx OR NOT kdtree_equi_probe OR
  no_idx IS DISTINCT FROM kdtree_seg_idx OR NOT kdtree_seg_probe OR
  no_idx IS DISTINCT FROM kdtree_bin_idx OR NOT kdtree_bin_probe
ORDER BY op;

DROP TABLE test_tjsonb_mest;
DROP TABLE tbl_tjsonb_span;
DROP TABLE tbl_tjsonb_val;
DROP TABLE tbl_tjsonb_src;
DROP TABLE tbl_tjsonb_idx;

DROP FUNCTION pg_temp.test_probes_index(text);

-------------------------------------------------------------------------------
