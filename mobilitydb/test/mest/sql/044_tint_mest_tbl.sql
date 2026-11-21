-------------------------------------------------------------------------------
-- The multi-entry R-tree mgist, quad-tree and k-d tree mspgist indexes of a tint,
-- keyed by the tboxes of the pieces of the value, one per split (equi, seg, tile).
-- Under each index the ever and always overlaps, with each right argument they
-- declare, answer the rows a sequential scan gives, from a plan that probes the
-- index.
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

DROP TABLE IF EXISTS tbl_tint_idx;
CREATE TEMP TABLE tbl_tint_idx AS
WITH base(k, temp) AS (
  SELECT k, temp FROM tbl_tint_big
  UNION ALL
  SELECT k + 10100, temp FROM tbl_tint_inst
  UNION ALL
  SELECT k + 10200, temp FROM tbl_tint_discseq
  UNION ALL
  SELECT k + 10300, temp FROM tbl_tint_contseq
  UNION ALL
  SELECT k + 10400, temp FROM tbl_tint_seqset )
SELECT k + n * 10500 AS k, shiftTime(temp, n * interval '1 week') AS temp
FROM base, generate_series(0, 0) AS n;
ANALYZE tbl_tint_idx;

-- The values, boxes, value spans and periods the operators are given. Those of
-- the fixture are random, so they meet few values exactly: the others come from
-- the rows of the table, so that every operator has rows to answer.
DROP TABLE IF EXISTS tbl_tint_src;
CREATE TEMP TABLE tbl_tint_src AS
SELECT k, temp, temp::tbox AS b, temp::tstzspan AS t, valueSpan(temp) AS v
FROM tbl_tint_idx WHERE k % 1000 = 0 AND temp IS NOT NULL;

-- A value of the fixture, a value of the table, and one whose period ends where
-- that of the value starts. The search for the nearest values, whose distance
-- costs more than a box, is given the second alone.
DROP TABLE IF EXISTS tbl_tint_val;
CREATE TEMP TABLE tbl_tint_val AS
SELECT temp, false AS knn FROM tbl_tint WHERE k % 20 = 0
UNION ALL
SELECT temp, true FROM tbl_tint_src
UNION ALL
SELECT shiftTime(temp, lower(t) - upper(t)), false FROM tbl_tint_src;
ANALYZE tbl_tint_val;

-- The box of a value of the fixture, the box of a value, one around it, one
-- inside it, and one whose period meets it
DROP TABLE IF EXISTS tbl_tint_box;
CREATE TEMP TABLE tbl_tint_box AS
SELECT temp::tbox AS b, false AS knn FROM tbl_tint WHERE k % 20 = 0
UNION ALL
SELECT b, true FROM tbl_tint_src
UNION ALL
SELECT expandTime(expandValue(b, 1::integer), interval '1 day'), false FROM tbl_tint_src
UNION ALL
SELECT tbox(format('[%s, %s]', (lower(v) + upper(v)) / 2, (lower(v) + upper(v)) / 2)::intspan,
    tstzspan(format('[%s, %s]', lower(t) + (upper(t) - lower(t)) / 2,
      lower(t) + (upper(t) - lower(t)) / 2))), false
FROM tbl_tint_src
UNION ALL
SELECT tbox(v, tstzspan(format('[%s, %s%s', lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END))), false
FROM tbl_tint_src;
ANALYZE tbl_tint_box;

-- A box of values alone, which the operators on time refuse: one around a
-- value, one inside it, the values of a value, and one that meets them
DROP TABLE IF EXISTS tbl_tint_xbox;
CREATE TEMP TABLE tbl_tint_xbox AS
SELECT tbox(format('[%s, %s]', lower(v) - 1::integer, upper(v) + 1::integer)::intspan) AS b
FROM tbl_tint_src
UNION ALL
SELECT tbox(format('[%s, %s]', (lower(v) + upper(v)) / 2, (lower(v) + upper(v)) / 2)::intspan) AS b
FROM tbl_tint_src
UNION ALL
SELECT tbox(v) AS b
FROM tbl_tint_src
UNION ALL
SELECT tbox(format('[%s, %s]', upper(v), upper(v) + 1::integer)::intspan) AS b
FROM tbl_tint_src;
ANALYZE tbl_tint_xbox;

-- The values of a value of the fixture, the values of a value, a span around
-- them, one inside them, and one that meets them
DROP TABLE IF EXISTS tbl_tint_vspan;
CREATE TEMP TABLE tbl_tint_vspan AS
SELECT valueSpan(temp) AS v, false AS knn FROM tbl_tint WHERE k % 20 = 0
UNION ALL
SELECT v, true FROM tbl_tint_src
UNION ALL
SELECT format('[%s, %s]', lower(v) - 1::integer, upper(v) + 1::integer)::intspan, false
FROM tbl_tint_src
UNION ALL
SELECT format('[%s, %s]', (lower(v) + upper(v)) / 2, (lower(v) + upper(v)) / 2)::intspan, false
FROM tbl_tint_src
UNION ALL
SELECT format('[%s, %s]', upper(v), upper(v) + 1::integer)::intspan, false
FROM tbl_tint_src;
ANALYZE tbl_tint_vspan;

-- A period of the fixture, the period of a value, one around it, one inside it,
-- and one that meets it
DROP TABLE IF EXISTS tbl_tint_span;
CREATE TEMP TABLE tbl_tint_span AS
SELECT t FROM tbl_tstzspan WHERE k % 20 = 0
UNION ALL
SELECT t FROM tbl_tint_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) - interval '1 day', upper(t) + interval '1 day'))
FROM tbl_tint_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2))
FROM tbl_tint_src
UNION ALL
SELECT tstzspan(format('[%s, %s%s', lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END))
FROM tbl_tint_src;
ANALYZE tbl_tint_span;

DROP TABLE IF EXISTS test_tint_mest;
CREATE TEMP TABLE test_tint_mest(op text, no_idx text,
  rtree_equi_idx text, rtree_equi_probe boolean,
  rtree_seg_idx text, rtree_seg_probe boolean,
  rtree_tile_idx text, rtree_tile_probe boolean,
  quadtree_equi_idx text, quadtree_equi_probe boolean,
  quadtree_seg_idx text, quadtree_seg_probe boolean,
  quadtree_tile_idx text, quadtree_tile_probe boolean,
  kdtree_equi_idx text, kdtree_equi_probe boolean,
  kdtree_seg_idx text, kdtree_seg_probe boolean,
  kdtree_tile_idx text, kdtree_tile_probe boolean);

INSERT INTO test_tint_mest(op, no_idx) VALUES ('?&& tbox', (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b)::text);
INSERT INTO test_tint_mest(op, no_idx) VALUES ('%&& tbox', (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b)::text);

-------------------------------------------------------------------------------

CREATE INDEX tbl_tint_idx_rtree_equi ON tbl_tint_idx
  USING mgist(temp tint_mrtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tint_idx_rtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tint_idx_rtree_seg ON tbl_tint_idx
  USING mgist(temp tint_mrtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tint_idx_rtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tint_idx_rtree_tile ON tbl_tint_idx
  USING mgist(temp tint_mrtree_tilesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tint_mest SET rtree_tile_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_tile_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tint_mest SET rtree_tile_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b)::text,
  rtree_tile_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tint_idx_rtree_tile;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tint_idx_quadtree_equi ON tbl_tint_idx
  USING mspgist(temp tint_mquadtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tint_idx_quadtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tint_idx_quadtree_seg ON tbl_tint_idx
  USING mspgist(temp tint_mquadtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tint_idx_quadtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tint_idx_quadtree_tile ON tbl_tint_idx
  USING mspgist(temp tint_mquadtree_tilesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tint_mest SET quadtree_tile_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_tile_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tint_mest SET quadtree_tile_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_tile_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tint_idx_quadtree_tile;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tint_idx_kdtree_equi ON tbl_tint_idx
  USING mspgist(temp tint_mkdtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tint_idx_kdtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tint_idx_kdtree_seg ON tbl_tint_idx
  USING mspgist(temp tint_mkdtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tint_idx_kdtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tint_idx_kdtree_tile ON tbl_tint_idx
  USING mspgist(temp tint_mkdtree_tilesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tint_mest SET kdtree_tile_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_tile_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tint_mest SET kdtree_tile_idx = (SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_tile_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tint_idx t1, tbl_tint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tint_idx_kdtree_tile;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint_idx;
SELECT COUNT(*) FROM test_tint_mest;
SELECT op, no_idx FROM test_tint_mest ORDER BY op;
SELECT * FROM test_tint_mest
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

DROP TABLE test_tint_mest;
DROP TABLE tbl_tint_span;
DROP TABLE tbl_tint_vspan;
DROP TABLE tbl_tint_xbox;
DROP TABLE tbl_tint_box;
DROP TABLE tbl_tint_val;
DROP TABLE tbl_tint_src;
DROP TABLE tbl_tint_idx;

DROP FUNCTION pg_temp.test_probes_index(text);

-------------------------------------------------------------------------------
