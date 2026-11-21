-------------------------------------------------------------------------------
-- The multi-entry R-tree mgist, quad-tree and k-d tree mspgist indexes of a
-- tnpoint, keyed by the stboxes of the pieces of the value, one per split (equi,
-- seg, merge, linear, adapt). Under each index the ever and always overlaps, with
-- each right argument they declare, answer the rows a sequential scan gives, from
-- a plan that probes the index.
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

DROP TABLE IF EXISTS tbl_tnpoint_idx;
CREATE TEMP TABLE tbl_tnpoint_idx AS
WITH base(k, temp) AS (
  SELECT k, temp FROM tbl_tnpoint_big
  UNION ALL
  SELECT k + 10100, temp FROM tbl_tnpoint_inst
  UNION ALL
  SELECT k + 10200, temp FROM tbl_tnpoint_discseq
  UNION ALL
  SELECT k + 10300, temp FROM tbl_tnpoint_contseq
  UNION ALL
  SELECT k + 10400, temp FROM tbl_tnpoint_seqset )
SELECT k + n * 10500 AS k, shiftTime(temp, n * interval '1 week') AS temp
FROM base, generate_series(0, 0) AS n;
ANALYZE tbl_tnpoint_idx;

-- The values, boxes and periods the operators are given. Those of the fixture
-- are random, so they meet few values exactly: the others come from the rows of
-- the table, so that every operator has rows to answer.
DROP TABLE IF EXISTS tbl_tnpoint_src;
CREATE TEMP TABLE tbl_tnpoint_src AS
SELECT k, temp, temp::stbox AS b, temp::tstzspan AS t
FROM tbl_tnpoint_idx WHERE k % 1000 = 0 AND temp IS NOT NULL;

-- A value of the fixture, a value of the table, and one whose period ends where
-- that of the value starts. The search for the nearest values, whose distance
-- costs more than a box, is given the second alone.
DROP TABLE IF EXISTS tbl_tnpoint_val;
CREATE TEMP TABLE tbl_tnpoint_val AS
SELECT temp, false AS knn FROM tbl_tnpoint WHERE k % 20 = 0
UNION ALL
SELECT temp, true FROM tbl_tnpoint_src
UNION ALL
SELECT shiftTime(temp, lower(t) - upper(t)), false FROM tbl_tnpoint_src;
ANALYZE tbl_tnpoint_val;

-- The box of a value of the fixture, the box of a value, one around it, one
-- inside it, and one whose period meets it
DROP TABLE IF EXISTS tbl_tnpoint_box;
CREATE TEMP TABLE tbl_tnpoint_box AS
SELECT temp::stbox AS b, false AS knn FROM tbl_tnpoint WHERE k % 20 = 0
UNION ALL
SELECT b, true FROM tbl_tnpoint_src
UNION ALL
SELECT expandTime(expandSpace(b, 0.1), interval '1 day'), false FROM tbl_tnpoint_src
UNION ALL
SELECT stbox(format('SRID=5676;STBOX XT(((%s,%s),(%s,%s)),[%s,%s])',
    (xmin(b) + xmax(b)) / 2, (ymin(b) + ymax(b)) / 2, (xmin(b) + xmax(b)) / 2 + 0.000001, (ymin(b) + ymax(b)) / 2 + 0.000001,
    lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2)), false
FROM tbl_tnpoint_src
UNION ALL
SELECT stbox(format('SRID=5676;STBOX XT(((%s,%s),(%s,%s)),[%s,%s%s)',
    xmin(b), ymin(b), xmax(b), ymax(b), lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END)), false
FROM tbl_tnpoint_src;
ANALYZE tbl_tnpoint_box;

-- A box of space alone, which the operators on time refuse: one around a value,
-- one inside it, the space of a value, and one that meets it
DROP TABLE IF EXISTS tbl_tnpoint_xbox;
CREATE TEMP TABLE tbl_tnpoint_xbox AS
SELECT stbox(format('SRID=5676;STBOX X((%s,%s),(%s,%s))',
    xmin(b) - 0.1, ymin(b) - 0.1, xmax(b) + 0.1, ymax(b) + 0.1)) AS b
FROM tbl_tnpoint_src
UNION ALL
SELECT stbox(format('SRID=5676;STBOX X((%s,%s),(%s,%s))',
    (xmin(b) + xmax(b)) / 2, (ymin(b) + ymax(b)) / 2, (xmin(b) + xmax(b)) / 2 + 0.000001, (ymin(b) + ymax(b)) / 2 + 0.000001)) AS b
FROM tbl_tnpoint_src
UNION ALL
SELECT stbox(format('SRID=5676;STBOX X((%s,%s),(%s,%s))',
    xmin(b), ymin(b), xmax(b), ymax(b))) AS b
FROM tbl_tnpoint_src
UNION ALL
SELECT stbox(format('SRID=5676;STBOX X((%s,%s),(%s,%s))',
    xmax(b), ymin(b), xmax(b) + 0.1, ymax(b))) AS b
FROM tbl_tnpoint_src;
ANALYZE tbl_tnpoint_xbox;

-- A period of the fixture, the period of a value, one around it, one inside it,
-- and one that meets it
DROP TABLE IF EXISTS tbl_tnpoint_span;
CREATE TEMP TABLE tbl_tnpoint_span AS
SELECT t FROM tbl_tstzspan WHERE k % 20 = 0
UNION ALL
SELECT t FROM tbl_tnpoint_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) - interval '1 day', upper(t) + interval '1 day'))
FROM tbl_tnpoint_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2))
FROM tbl_tnpoint_src
UNION ALL
SELECT tstzspan(format('[%s, %s%s', lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END))
FROM tbl_tnpoint_src;
ANALYZE tbl_tnpoint_span;

-- A point at a corner of the box of a value
DROP TABLE IF EXISTS tbl_tnpoint_geo;
CREATE TEMP TABLE tbl_tnpoint_geo AS
SELECT ST_SetSRID(ST_MakePoint(xmin(b), ymin(b)), 5676) AS g, true AS knn FROM tbl_tnpoint_src;
ANALYZE tbl_tnpoint_geo;

DROP TABLE IF EXISTS test_tnpoint_mest;
CREATE TEMP TABLE test_tnpoint_mest(op text, no_idx text,
  rtree_equi_idx text, rtree_equi_probe boolean,
  rtree_seg_idx text, rtree_seg_probe boolean,
  rtree_merge_idx text, rtree_merge_probe boolean,
  rtree_linear_idx text, rtree_linear_probe boolean,
  rtree_adapt_idx text, rtree_adapt_probe boolean,
  quadtree_equi_idx text, quadtree_equi_probe boolean,
  quadtree_seg_idx text, quadtree_seg_probe boolean,
  quadtree_merge_idx text, quadtree_merge_probe boolean,
  quadtree_linear_idx text, quadtree_linear_probe boolean,
  quadtree_adapt_idx text, quadtree_adapt_probe boolean,
  kdtree_equi_idx text, kdtree_equi_probe boolean,
  kdtree_seg_idx text, kdtree_seg_probe boolean,
  kdtree_merge_idx text, kdtree_merge_probe boolean,
  kdtree_linear_idx text, kdtree_linear_probe boolean,
  kdtree_adapt_idx text, kdtree_adapt_probe boolean);

INSERT INTO test_tnpoint_mest(op, no_idx) VALUES ('?&& stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text);
INSERT INTO test_tnpoint_mest(op, no_idx) VALUES ('%&& stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text);

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_rtree_equi ON tbl_tnpoint_idx
  USING mgist(temp tnpoint_mrtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_rtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_rtree_seg ON tbl_tnpoint_idx
  USING mgist(temp tnpoint_mrtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_rtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_rtree_merge ON tbl_tnpoint_idx
  USING mgist(temp tnpoint_mrtree_mergesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_rtree_merge;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_rtree_linear ON tbl_tnpoint_idx
  USING mgist(temp tnpoint_mrtree_linearsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET rtree_linear_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_linear_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET rtree_linear_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  rtree_linear_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_rtree_linear;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_rtree_adapt ON tbl_tnpoint_idx
  USING mgist(temp tnpoint_mrtree_adaptsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_rtree_adapt;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_quadtree_equi ON tbl_tnpoint_idx
  USING mspgist(temp tnpoint_mquadtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_quadtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_quadtree_seg ON tbl_tnpoint_idx
  USING mspgist(temp tnpoint_mquadtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_quadtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_quadtree_merge ON tbl_tnpoint_idx
  USING mspgist(temp tnpoint_mquadtree_mergesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_quadtree_merge;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_quadtree_linear ON tbl_tnpoint_idx
  USING mspgist(temp tnpoint_mquadtree_linearsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET quadtree_linear_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_linear_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET quadtree_linear_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_linear_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_quadtree_linear;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_quadtree_adapt ON tbl_tnpoint_idx
  USING mspgist(temp tnpoint_mquadtree_adaptsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_quadtree_adapt;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_kdtree_equi ON tbl_tnpoint_idx
  USING mspgist(temp tnpoint_mkdtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_kdtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_kdtree_seg ON tbl_tnpoint_idx
  USING mspgist(temp tnpoint_mkdtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_kdtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_kdtree_merge ON tbl_tnpoint_idx
  USING mspgist(temp tnpoint_mkdtree_mergesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_kdtree_merge;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_kdtree_linear ON tbl_tnpoint_idx
  USING mspgist(temp tnpoint_mkdtree_linearsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET kdtree_linear_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_linear_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET kdtree_linear_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_linear_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_kdtree_linear;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_kdtree_adapt ON tbl_tnpoint_idx
  USING mspgist(temp tnpoint_mkdtree_adaptsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tnpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_kdtree_adapt;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tnpoint_idx;
SELECT COUNT(*) FROM test_tnpoint_mest;
SELECT op, no_idx FROM test_tnpoint_mest ORDER BY op;
SELECT * FROM test_tnpoint_mest
WHERE no_idx IS DISTINCT FROM rtree_equi_idx OR NOT rtree_equi_probe OR
  no_idx IS DISTINCT FROM rtree_seg_idx OR NOT rtree_seg_probe OR
  no_idx IS DISTINCT FROM rtree_merge_idx OR NOT rtree_merge_probe OR
  no_idx IS DISTINCT FROM rtree_linear_idx OR NOT rtree_linear_probe OR
  no_idx IS DISTINCT FROM rtree_adapt_idx OR NOT rtree_adapt_probe OR
  no_idx IS DISTINCT FROM quadtree_equi_idx OR NOT quadtree_equi_probe OR
  no_idx IS DISTINCT FROM quadtree_seg_idx OR NOT quadtree_seg_probe OR
  no_idx IS DISTINCT FROM quadtree_merge_idx OR NOT quadtree_merge_probe OR
  no_idx IS DISTINCT FROM quadtree_linear_idx OR NOT quadtree_linear_probe OR
  no_idx IS DISTINCT FROM quadtree_adapt_idx OR NOT quadtree_adapt_probe OR
  no_idx IS DISTINCT FROM kdtree_equi_idx OR NOT kdtree_equi_probe OR
  no_idx IS DISTINCT FROM kdtree_seg_idx OR NOT kdtree_seg_probe OR
  no_idx IS DISTINCT FROM kdtree_merge_idx OR NOT kdtree_merge_probe OR
  no_idx IS DISTINCT FROM kdtree_linear_idx OR NOT kdtree_linear_probe OR
  no_idx IS DISTINCT FROM kdtree_adapt_idx OR NOT kdtree_adapt_probe
ORDER BY op;

DROP TABLE test_tnpoint_mest;
DROP TABLE tbl_tnpoint_geo;
DROP TABLE tbl_tnpoint_span;
DROP TABLE tbl_tnpoint_xbox;
DROP TABLE tbl_tnpoint_box;
DROP TABLE tbl_tnpoint_val;
DROP TABLE tbl_tnpoint_src;
DROP TABLE tbl_tnpoint_idx;

DROP FUNCTION pg_temp.test_probes_index(text);

-------------------------------------------------------------------------------
