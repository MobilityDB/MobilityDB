-------------------------------------------------------------------------------
-- The R-tree GiST, quad-tree and k-d tree SP-GiST indexes of a tbool, keyed by the
-- tstzspan of the value. Under each index every operator of the class, with each
-- right argument it declares, answers the rows a sequential scan gives, from a
-- plan that probes the index.
-------------------------------------------------------------------------------

CREATE FUNCTION pg_temp.test_probes_index(query text)
RETURNS boolean LANGUAGE plpgsql AS $$
DECLARE
  line text;
BEGIN
  FOR line IN EXECUTE 'EXPLAIN (COSTS OFF) ' || query LOOP
    IF line LIKE '%Index Cond%' OR line LIKE '%Order By%' THEN
      RETURN true;
    END IF;
  END LOOP;
  RETURN false;
END;
$$;

DROP TABLE IF EXISTS tbl_tbool_idx;
CREATE TEMP TABLE tbl_tbool_idx AS
WITH base(k, temp) AS (
  SELECT k, temp FROM tbl_tbool_big
  UNION ALL
  SELECT k + 10000, temp FROM tbl_tbool_inst
  UNION ALL
  SELECT k + 10100, temp FROM tbl_tbool_discseq
  UNION ALL
  SELECT k + 10200, temp FROM tbl_tbool_contseq
  UNION ALL
  SELECT k + 10300, temp FROM tbl_tbool_seqset )
SELECT k + n * 10400 AS k, shiftTime(temp, n * interval '1 week') AS temp
FROM base, generate_series(0, 0) AS n;
ANALYZE tbl_tbool_idx;

-- The values and periods the operators are given. Those of the fixture are
-- random, so they meet few values exactly: the others come from the rows of the
-- table, so that every operator has rows to answer.
DROP TABLE IF EXISTS tbl_tbool_src;
CREATE TEMP TABLE tbl_tbool_src AS
SELECT k, temp, temp::tstzspan AS t
FROM tbl_tbool_idx WHERE k % 1000 = 0 AND temp IS NOT NULL;

-- A value of the fixture, a value of the table, and one whose period ends where
-- that of the value starts
DROP TABLE IF EXISTS tbl_tbool_val;
CREATE TEMP TABLE tbl_tbool_val AS
SELECT temp, false AS knn FROM tbl_tbool WHERE k % 20 = 0
UNION ALL
SELECT temp, true FROM tbl_tbool_src
UNION ALL
SELECT shiftTime(temp, lower(t) - upper(t)), false FROM tbl_tbool_src;
ANALYZE tbl_tbool_val;

-- A period of the fixture, the period of a value, one around it, one inside it,
-- and one that meets it
DROP TABLE IF EXISTS tbl_tbool_span;
CREATE TEMP TABLE tbl_tbool_span AS
SELECT t FROM tbl_tstzspan WHERE k % 20 = 0
UNION ALL
SELECT t FROM tbl_tbool_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) - interval '1 day', upper(t) + interval '1 day'))
FROM tbl_tbool_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2))
FROM tbl_tbool_src
UNION ALL
SELECT tstzspan(format('[%s, %s%s', lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END))
FROM tbl_tbool_src;
ANALYZE tbl_tbool_span;

DROP TABLE IF EXISTS test_tbool_idx;
CREATE TEMP TABLE test_tbool_idx(op text, no_idx text, rtree_idx text, quadtree_idx text,
  kdtree_idx text, rtree_probe boolean, quadtree_probe boolean,
  kdtree_probe boolean);

INSERT INTO test_tbool_idx(op, no_idx) VALUES ('&& tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp && t2.t)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('&& tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('~= tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('~= tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('@> tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp @> t2.t)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('@> tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp @> t2.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('<@ tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp <@ t2.t)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('<@ tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('-|- tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp -|- t2.t)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('-|- tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('&<# tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('&<# tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('<<# tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('<<# tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('#>> tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('#>> tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('#&> tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('#&> tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('?&& tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp ?&& t2.t)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('%&& tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp %&& t2.t)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('tstzspan && tbool', (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t && t1.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('tstzspan ~= tbool', (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t ~= t1.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('tstzspan <@ tbool', (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t <@ t1.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('tstzspan @> tbool', (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t @> t1.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('tstzspan -|- tbool', (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t -|- t1.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('tstzspan #>> tbool', (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t #>> t1.temp)::text);
INSERT INTO test_tbool_idx(op, no_idx) VALUES ('tstzspan <<# tbool', (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t <<# t1.temp)::text);

-- With no index a plan cannot probe one, whatever the planner settings
SET enable_seqscan = off;
SELECT pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp && t2.t') AS no_index_probe;
RESET enable_seqscan;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_idx_rtree ON tbl_tbool_idx USING gist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp && t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp && t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp ~= t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp @> t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp @> t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp <@ t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp -|- t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp &<# t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp <<# t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp #>> t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp #&> t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp ?&& t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp ?&& t2.t')
WHERE op = '?&& tstzspan';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp %&& t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp %&& t2.t')
WHERE op = '%&& tstzspan';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t && t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t ~= t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t <@ t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t @> t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t @> t1.temp')
WHERE op = 'tstzspan @> tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t -|- t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t #>> t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tbool';
UPDATE test_tbool_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t <<# t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tbool';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbool_idx_rtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_idx_quadtree ON tbl_tbool_idx USING spgist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp && t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp && t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp @> t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp ?&& t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp ?&& t2.t')
WHERE op = '?&& tstzspan';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp %&& t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp %&& t2.t')
WHERE op = '%&& tstzspan';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t && t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t ~= t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t <@ t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t @> t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t @> t1.temp')
WHERE op = 'tstzspan @> tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t -|- t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t #>> t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tbool';
UPDATE test_tbool_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t <<# t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tbool';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbool_idx_quadtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_idx_kdtree ON tbl_tbool_idx USING spgist(temp tbool_kdtree_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp && t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp && t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp @> t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp ?&& t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp ?&& t2.t')
WHERE op = '?&& tstzspan';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp %&& t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool_span t2 WHERE t1.temp %&& t2.t')
WHERE op = '%&& tstzspan';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t && t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t ~= t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t <@ t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t @> t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t @> t1.temp')
WHERE op = 'tstzspan @> tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t -|- t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t #>> t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tbool';
UPDATE test_tbool_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t <<# t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_span t2, tbl_tbool_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tbool';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbool_idx_kdtree;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool_idx;
SELECT COUNT(*) FROM test_tbool_idx;
SELECT op, no_idx FROM test_tbool_idx WHERE op NOT LIKE '%|=|%' ORDER BY op;
-- The distances of the nearest values are compared, not stated
SELECT op FROM test_tbool_idx WHERE op LIKE '%|=|%' AND no_idx IS NULL ORDER BY op;
SELECT * FROM test_tbool_idx
WHERE no_idx IS DISTINCT FROM rtree_idx OR no_idx IS DISTINCT FROM quadtree_idx OR
  no_idx IS DISTINCT FROM kdtree_idx OR NOT rtree_probe OR NOT quadtree_probe OR
  NOT kdtree_probe
ORDER BY op;

DROP TABLE test_tbool_idx;
DROP TABLE tbl_tbool_span;
DROP TABLE tbl_tbool_val;
DROP TABLE tbl_tbool_src;
DROP TABLE tbl_tbool_idx;


DROP FUNCTION pg_temp.test_probes_index(text);

-------------------------------------------------------------------------------

