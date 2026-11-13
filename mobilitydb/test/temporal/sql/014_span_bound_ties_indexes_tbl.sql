-------------------------------------------------------------------------------
-- File span_spgist.c
-- Tests of the SP-GiST indexes of span types on spans sharing their bounds:
-- the lower bounds take 23 values and the upper bounds 5 widths above them,
-- each bound inclusive or exclusive, so most spans tie on a bound value with
-- another span that differs from it only by the inclusivity of that bound.
-- For each table size and query span, the count of each operator through the
-- quad tree and the k-d tree must equal the count without an index.
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_floatspan_ties, tbl_floatspan_ties_q,
  tbl_tstzspan_ties, tbl_tstzspan_ties_q, ties_noidx, ties_idx, ties_plan;

CREATE TABLE tbl_floatspan_ties(k int, f floatspan);
CREATE TABLE tbl_tstzspan_ties(k int, f tstzspan);
CREATE TABLE tbl_floatspan_ties_q AS
SELECT k, (CASE WHEN k % 3 = 0 THEN '[' ELSE '(' END || (k % 23) || ', ' ||
  (k % 23 + 1 + (k * 3) % 5) || CASE WHEN k % 5 < 2 THEN ']' ELSE ')' END)::floatspan AS f
FROM generate_series(1, 120) k;
CREATE TABLE tbl_tstzspan_ties_q AS
SELECT k, span(timestamptz 'epoch' + (11323 + k % 23) * interval '1 day',
  timestamptz 'epoch' + (11323 + k % 23 + 1 + (k * 3) % 5) * interval '1 day',
  k % 3 = 0, k % 5 < 2) AS f
FROM generate_series(1, 120) k;

-- The count of the rows answering each query span through an operator
CREATE OR REPLACE FUNCTION ties_counts(tbl text, op text)
RETURNS TABLE(k int, c bigint) AS $$
BEGIN
  RETURN QUERY EXECUTE format(
    'SELECT q.k, (SELECT COUNT(*) FROM %I s WHERE s.f %s q.f) FROM %I q',
    tbl, op, tbl || '_q');
END $$ LANGUAGE plpgsql;

-- Whether the plan of the first query span reads the index
CREATE OR REPLACE FUNCTION ties_uses_index(tbl text, op text)
RETURNS bool AS $$
DECLARE
  line text;
  found bool := false;
BEGIN
  FOR line IN EXECUTE format(
    'EXPLAIN SELECT COUNT(*) FROM %I s WHERE s.f %s (SELECT f FROM %I WHERE k = 1)',
    tbl, op, tbl || '_q')
  LOOP
    IF line LIKE '%Index%' THEN found := true; END IF;
  END LOOP;
  RETURN found;
END $$ LANGUAGE plpgsql;

CREATE TABLE ties_noidx(n int, tbl text, op text, k int, c bigint);
CREATE TABLE ties_idx(n int, idx text, tbl text, op text, k int, c bigint);
CREATE TABLE ties_plan(n int, idx text, tbl text, op text, uses_index bool);
CREATE TEMP TABLE ties_ops(tbl text, op text);
INSERT INTO ties_ops
SELECT 'tbl_floatspan_ties', op
FROM unnest(ARRAY['&&', '@>', '<@', '-|-', '<<', '&<', '>>', '&>', '=']) op
UNION ALL
SELECT 'tbl_tstzspan_ties', op
FROM unnest(ARRAY['&&', '@>', '<@', '-|-', '<<#', '&<#', '#>>', '#&>', '=']) op;

-- The counts for one table size, without an index and through each index
CREATE OR REPLACE FUNCTION ties_run(n int)
RETURNS void AS $$
BEGIN
  TRUNCATE tbl_floatspan_ties, tbl_tstzspan_ties;
  INSERT INTO tbl_floatspan_ties
  SELECT k, (CASE WHEN k % 2 = 0 THEN '[' ELSE '(' END || (k % 23) || ', ' ||
    (k % 23 + 1 + (k * 7) % 5) || CASE WHEN (k / 2) % 2 = 0 THEN ']' ELSE ')' END)::floatspan
  FROM generate_series(1, n) k;
  INSERT INTO tbl_tstzspan_ties
  SELECT k, span(timestamptz 'epoch' + (11323 + k % 23) * interval '1 day',
    timestamptz 'epoch' + (11323 + k % 23 + 1 + (k * 7) % 5) * interval '1 day',
    k % 2 = 0, (k / 2) % 2 = 0)
  FROM generate_series(1, n) k;
  ANALYZE tbl_floatspan_ties;
  ANALYZE tbl_tstzspan_ties;

  SET LOCAL enable_indexscan = off;
  SET LOCAL enable_bitmapscan = off;
  INSERT INTO ties_noidx
  SELECT n, o.tbl, o.op, r.k, r.c FROM ties_ops o, LATERAL ties_counts(o.tbl, o.op) r;
  SET LOCAL enable_indexscan = on;
  SET LOCAL enable_bitmapscan = on;
  SET LOCAL enable_seqscan = off;

  CREATE INDEX tbl_floatspan_ties_quadtree_idx ON tbl_floatspan_ties USING SPGIST(f);
  CREATE INDEX tbl_tstzspan_ties_quadtree_idx ON tbl_tstzspan_ties USING SPGIST(f);
  INSERT INTO ties_plan
  SELECT n, 'quadtree', o.tbl, o.op, ties_uses_index(o.tbl, o.op) FROM ties_ops o;
  INSERT INTO ties_idx
  SELECT n, 'quadtree', o.tbl, o.op, r.k, r.c FROM ties_ops o, LATERAL ties_counts(o.tbl, o.op) r;
  DROP INDEX tbl_floatspan_ties_quadtree_idx;
  DROP INDEX tbl_tstzspan_ties_quadtree_idx;

  CREATE INDEX tbl_floatspan_ties_kdtree_idx ON tbl_floatspan_ties USING SPGIST(f floatspan_kdtree_ops);
  CREATE INDEX tbl_tstzspan_ties_kdtree_idx ON tbl_tstzspan_ties USING SPGIST(f tstzspan_kdtree_ops);
  INSERT INTO ties_plan
  SELECT n, 'kdtree', o.tbl, o.op, ties_uses_index(o.tbl, o.op) FROM ties_ops o;
  INSERT INTO ties_idx
  SELECT n, 'kdtree', o.tbl, o.op, r.k, r.c FROM ties_ops o, LATERAL ties_counts(o.tbl, o.op) r;
  DROP INDEX tbl_floatspan_ties_kdtree_idx;
  DROP INDEX tbl_tstzspan_ties_kdtree_idx;
END $$ LANGUAGE plpgsql;

SELECT ties_run(n) FROM unnest(ARRAY[300, 1200, 2400, 5000]) n;

-- Every plan reads its index
SELECT COUNT(*) AS plans, COUNT(*) FILTER (WHERE uses_index) AS plans_with_index
FROM ties_plan;

-- Per index, table and operator, over every table size: the query spans, the
-- rows they answer without an index, and the query spans whose count through
-- the index differs
SELECT i.idx, i.tbl, i.op, COUNT(*) AS queries, SUM(n.c) AS rows_noidx,
  COUNT(*) FILTER (WHERE i.c <> n.c) AS queries_differ,
  SUM(n.c - i.c) AS rows_missing
FROM ties_idx i JOIN ties_noidx n USING (n, tbl, op, k)
GROUP BY i.idx, i.tbl, i.op
ORDER BY i.idx, i.tbl, i.op;

DROP FUNCTION ties_run(int);
DROP FUNCTION ties_uses_index(text, text);
DROP FUNCTION ties_counts(text, text);
DROP TABLE tbl_floatspan_ties, tbl_floatspan_ties_q, tbl_tstzspan_ties,
  tbl_tstzspan_ties_q, ties_noidx, ties_idx, ties_plan;
