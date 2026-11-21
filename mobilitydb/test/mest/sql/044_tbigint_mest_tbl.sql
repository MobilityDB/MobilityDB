-------------------------------------------------------------------------------
-- The multi-entry R-tree mgist, quad-tree and k-d tree mspgist indexes of a
-- tbigint, keyed by the tboxes of the pieces of the value, one per split (equi,
-- seg, tile). Under each index the ever and always overlaps, with each right
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

DROP TABLE IF EXISTS tbl_tbigint_idx;
CREATE TEMP TABLE tbl_tbigint_idx AS
WITH base(k, temp) AS (
  SELECT k, temp FROM tbl_tbigint_big
  UNION ALL
  SELECT k + 10100, temp FROM tbl_tbigint_inst
  UNION ALL
  SELECT k + 10200, temp FROM tbl_tbigint_discseq
  UNION ALL
  SELECT k + 10300, temp FROM tbl_tbigint_contseq
  UNION ALL
  SELECT k + 10400, temp FROM tbl_tbigint_seqset )
SELECT k + n * 10500 AS k, shiftTime(temp, n * interval '1 week') AS temp
FROM base, generate_series(0, 0) AS n;
ANALYZE tbl_tbigint_idx;

-- The values, boxes, value spans and periods the operators are given. Those of
-- the fixture are random, so they meet few values exactly: the others come from
-- the rows of the table, so that every operator has rows to answer.
DROP TABLE IF EXISTS tbl_tbigint_src;
CREATE TEMP TABLE tbl_tbigint_src AS
SELECT k, temp, temp::tbox AS b, temp::tstzspan AS t, valueSpan(temp) AS v
FROM tbl_tbigint_idx WHERE k % 1000 = 0 AND temp IS NOT NULL;

-- A value of the fixture, a value of the table, and one whose period ends where
-- that of the value starts. The search for the nearest values, whose distance
-- costs more than a box, is given the second alone.
DROP TABLE IF EXISTS tbl_tbigint_val;
CREATE TEMP TABLE tbl_tbigint_val AS
SELECT temp, false AS knn FROM tbl_tbigint WHERE k % 20 = 0
UNION ALL
SELECT temp, true FROM tbl_tbigint_src
UNION ALL
SELECT shiftTime(temp, lower(t) - upper(t)), false FROM tbl_tbigint_src;
ANALYZE tbl_tbigint_val;

-- The box of a value of the fixture, the box of a value, one around it, one
-- inside it, and one whose period meets it
DROP TABLE IF EXISTS tbl_tbigint_box;
CREATE TEMP TABLE tbl_tbigint_box AS
SELECT temp::tbox AS b, false AS knn FROM tbl_tbigint WHERE k % 20 = 0
UNION ALL
SELECT b, true FROM tbl_tbigint_src
UNION ALL
SELECT expandTime(expandValue(b, 1::bigint), interval '1 day'), false FROM tbl_tbigint_src
UNION ALL
SELECT tbox(format('[%s, %s]', (lower(v) + upper(v)) / 2, (lower(v) + upper(v)) / 2)::bigintspan,
    tstzspan(format('[%s, %s]', lower(t) + (upper(t) - lower(t)) / 2,
      lower(t) + (upper(t) - lower(t)) / 2))), false
FROM tbl_tbigint_src
UNION ALL
SELECT tbox(v, tstzspan(format('[%s, %s%s', lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END))), false
FROM tbl_tbigint_src;
ANALYZE tbl_tbigint_box;

-- A box of values alone, which the operators on time refuse: one around a
-- value, one inside it, the values of a value, and one that meets them
DROP TABLE IF EXISTS tbl_tbigint_xbox;
CREATE TEMP TABLE tbl_tbigint_xbox AS
SELECT tbox(format('[%s, %s]', lower(v) - 1::bigint, upper(v) + 1::bigint)::bigintspan) AS b
FROM tbl_tbigint_src
UNION ALL
SELECT tbox(format('[%s, %s]', (lower(v) + upper(v)) / 2, (lower(v) + upper(v)) / 2)::bigintspan) AS b
FROM tbl_tbigint_src
UNION ALL
SELECT tbox(v) AS b
FROM tbl_tbigint_src
UNION ALL
SELECT tbox(format('[%s, %s]', upper(v), upper(v) + 1::bigint)::bigintspan) AS b
FROM tbl_tbigint_src;
ANALYZE tbl_tbigint_xbox;

-- The values of a value of the fixture, the values of a value, a span around
-- them, one inside them, and one that meets them
DROP TABLE IF EXISTS tbl_tbigint_vspan;
CREATE TEMP TABLE tbl_tbigint_vspan AS
SELECT valueSpan(temp) AS v, false AS knn FROM tbl_tbigint WHERE k % 20 = 0
UNION ALL
SELECT v, true FROM tbl_tbigint_src
UNION ALL
SELECT format('[%s, %s]', lower(v) - 1::bigint, upper(v) + 1::bigint)::bigintspan, false
FROM tbl_tbigint_src
UNION ALL
SELECT format('[%s, %s]', (lower(v) + upper(v)) / 2, (lower(v) + upper(v)) / 2)::bigintspan, false
FROM tbl_tbigint_src
UNION ALL
SELECT format('[%s, %s]', upper(v), upper(v) + 1::bigint)::bigintspan, false
FROM tbl_tbigint_src;
ANALYZE tbl_tbigint_vspan;

-- A period of the fixture, the period of a value, one around it, one inside it,
-- and one that meets it
DROP TABLE IF EXISTS tbl_tbigint_span;
CREATE TEMP TABLE tbl_tbigint_span AS
SELECT t FROM tbl_tstzspan WHERE k % 20 = 0
UNION ALL
SELECT t FROM tbl_tbigint_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) - interval '1 day', upper(t) + interval '1 day'))
FROM tbl_tbigint_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2))
FROM tbl_tbigint_src
UNION ALL
SELECT tstzspan(format('[%s, %s%s', lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END))
FROM tbl_tbigint_src;
ANALYZE tbl_tbigint_span;

DROP TABLE IF EXISTS test_tbigint_mest;
CREATE TEMP TABLE test_tbigint_mest(op text, no_idx text,
  rtree_equi_idx text, rtree_equi_probe boolean,
  rtree_seg_idx text, rtree_seg_probe boolean,
  rtree_tile_idx text, rtree_tile_probe boolean,
  quadtree_equi_idx text, quadtree_equi_probe boolean,
  quadtree_seg_idx text, quadtree_seg_probe boolean,
  quadtree_tile_idx text, quadtree_tile_probe boolean,
  kdtree_equi_idx text, kdtree_equi_probe boolean,
  kdtree_seg_idx text, kdtree_seg_probe boolean,
  kdtree_tile_idx text, kdtree_tile_probe boolean);

INSERT INTO test_tbigint_mest(op, no_idx) VALUES ('?&& tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b)::text);
INSERT INTO test_tbigint_mest(op, no_idx) VALUES ('%&& tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b)::text);

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbigint_idx_rtree_equi ON tbl_tbigint_idx
  USING mgist(temp tbigint_mrtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbigint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tbigint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbigint_idx_rtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbigint_idx_rtree_seg ON tbl_tbigint_idx
  USING mgist(temp tbigint_mrtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbigint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tbigint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbigint_idx_rtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbigint_idx_rtree_tile ON tbl_tbigint_idx
  USING mgist(temp tbigint_mrtree_tilesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbigint_mest SET rtree_tile_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_tile_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tbigint_mest SET rtree_tile_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b)::text,
  rtree_tile_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbigint_idx_rtree_tile;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbigint_idx_quadtree_equi ON tbl_tbigint_idx
  USING mspgist(temp tbigint_mquadtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbigint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tbigint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbigint_idx_quadtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbigint_idx_quadtree_seg ON tbl_tbigint_idx
  USING mspgist(temp tbigint_mquadtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbigint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tbigint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbigint_idx_quadtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbigint_idx_quadtree_tile ON tbl_tbigint_idx
  USING mspgist(temp tbigint_mquadtree_tilesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbigint_mest SET quadtree_tile_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_tile_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tbigint_mest SET quadtree_tile_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_tile_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbigint_idx_quadtree_tile;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbigint_idx_kdtree_equi ON tbl_tbigint_idx
  USING mspgist(temp tbigint_mkdtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbigint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tbigint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbigint_idx_kdtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbigint_idx_kdtree_seg ON tbl_tbigint_idx
  USING mspgist(temp tbigint_mkdtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbigint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tbigint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbigint_idx_kdtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbigint_idx_kdtree_tile ON tbl_tbigint_idx
  USING mspgist(temp tbigint_mkdtree_tilesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbigint_mest SET kdtree_tile_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_tile_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tbigint_mest SET kdtree_tile_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_tile_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbigint_idx_kdtree_tile;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbigint_idx;
SELECT COUNT(*) FROM test_tbigint_mest;
SELECT op, no_idx FROM test_tbigint_mest ORDER BY op;
SELECT * FROM test_tbigint_mest
WHERE no_idx IS DISTINCT FROM rtree_equi_idx OR NOT rtree_equi_probe OR
  no_idx IS DISTINCT FROM rtree_seg_idx OR NOT rtree_seg_probe OR
  no_idx IS DISTINCT FROM rtree_tile_idx OR NOT rtree_tile_probe OR
  no_idx IS DISTINCT FROM quadtree_equi_idx OR NOT quadtree_equi_probe OR
  no_idx IS DISTINCT FROM quadtree_seg_idx OR NOT quadtree_seg_probe OR
  no_idx IS DISTINCT FROM quadtree_tile_idx OR NOT quadtree_tile_probe OR
  no_idx IS DISTINCT FROM kdtree_equi_idx OR NOT kdtree_equi_probe OR
  no_idx IS DISTINCT FROM kdtree_seg_idx OR NOT kdtree_seg_probe OR
  no_idx IS DISTINCT FROM kdtree_tile_idx OR NOT kdtree_tile_probe
ORDER BY op;

DROP TABLE test_tbigint_mest;
DROP TABLE tbl_tbigint_span;
DROP TABLE tbl_tbigint_vspan;
DROP TABLE tbl_tbigint_xbox;
DROP TABLE tbl_tbigint_box;
DROP TABLE tbl_tbigint_val;
DROP TABLE tbl_tbigint_src;
DROP TABLE tbl_tbigint_idx;

DROP FUNCTION pg_temp.test_probes_index(text);

-------------------------------------------------------------------------------
