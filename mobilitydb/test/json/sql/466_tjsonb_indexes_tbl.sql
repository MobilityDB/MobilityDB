-------------------------------------------------------------------------------
-- The R-tree GiST, quad-tree and k-d tree SP-GiST indexes of a tjsonb, keyed by
-- the period of the value. Under each index every operator of the class, with
-- each right argument it declares, answers the rows a sequential scan gives,
-- from a plan that probes the index.
-------------------------------------------------------------------------------

CREATE FUNCTION test_probes_index(query text)
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

-- The instants and sequences of the fixture, each repeated a week apart
DROP TABLE IF EXISTS tbl_tjsonb_idx;
CREATE TABLE tbl_tjsonb_idx AS
WITH base(k, temp) AS (
  SELECT k, inst FROM tbl_tjsonb_inst
  UNION ALL
  SELECT k + 100, seq FROM tbl_tjsonb_discseq
  UNION ALL
  SELECT k + 200, seq FROM tbl_tjsonb_seq
  UNION ALL
  SELECT k + 300, ss FROM tbl_tjsonb_seqset )
SELECT k + n * 400 AS k, shiftTime(temp, n * interval '1 week') AS temp
FROM base, generate_series(0, 25) AS n;
ANALYZE tbl_tjsonb_idx;

-- The values and periods the operators are given, drawn from the rows of the
-- table so that every operator has rows to answer. This step passes through the
-- instants and each kind of sequence in turn.
DROP TABLE IF EXISTS tbl_tjsonb_src;
CREATE TABLE tbl_tjsonb_src AS
SELECT k, temp, temp::tstzspan AS t
FROM tbl_tjsonb_idx WHERE k % 900 = 0;

-- A value of the table, and one whose period ends where that of the value starts
DROP TABLE IF EXISTS tbl_tjsonb_val;
CREATE TABLE tbl_tjsonb_val AS
SELECT temp FROM tbl_tjsonb_src
UNION ALL
SELECT shiftTime(temp, lower(t) - upper(t)) FROM tbl_tjsonb_src;
ANALYZE tbl_tjsonb_val;

-- A period of the fixture, the period of a value, one around it, one inside it,
-- and one that meets it
DROP TABLE IF EXISTS tbl_tjsonb_span;
CREATE TABLE tbl_tjsonb_span AS
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

DROP TABLE IF EXISTS test_tjsonb_idx;
CREATE TABLE test_tjsonb_idx(op text, no_idx text, rtree_idx text, quadtree_idx text,
  kdtree_idx text, rtree_probe boolean, quadtree_probe boolean,
  kdtree_probe boolean);

INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('&& tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('~= tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('#@> tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #@> t2.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('<@# tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp <@# t2.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('-|- tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('&<# tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('<<# tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('#>> tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('#&> tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('&& tstzspan', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp && t2.t)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('~= tstzspan', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('#@> tstzspan', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #@> t2.t)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('<@# tstzspan', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp <@# t2.t)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('-|- tstzspan', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp -|- t2.t)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('&<# tstzspan', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('<<# tstzspan', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('#>> tstzspan', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('#&> tstzspan', (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('tstzspan && tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t && t1.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('tstzspan ~= tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t ~= t1.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('tstzspan #@> tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t #@> t1.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('tstzspan <@# tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t <@# t1.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('tstzspan -|- tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t -|- t1.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('tstzspan <<# tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t <<# t1.temp)::text);
INSERT INTO test_tjsonb_idx(op, no_idx) VALUES ('tstzspan #>> tjsonb', (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t #>> t1.temp)::text);

-- With no index a plan cannot probe one, whatever the planner settings
SET enable_seqscan = off;
SELECT test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp && t2.t') AS no_index_probe;
RESET enable_seqscan;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tjsonb_idx_rtree ON tbl_tjsonb_idx USING gist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp && t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #@> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #@> t2.temp')
WHERE op = '#@> tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp <@# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp <@# t2.temp')
WHERE op = '<@# tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp && t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ~= t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #@> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #@> t2.t')
WHERE op = '#@> tstzspan';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp <@# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp <@# t2.t')
WHERE op = '<@# tstzspan';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp -|- t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp &<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp <<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #>> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #&> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t && t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t ~= t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t #@> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t #@> t1.temp')
WHERE op = 'tstzspan #@> tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t <@# t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t <@# t1.temp')
WHERE op = 'tstzspan <@# tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t -|- t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t <<# t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tjsonb';
UPDATE test_tjsonb_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t #>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tjsonb';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tjsonb_idx_rtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tjsonb_idx_quadtree ON tbl_tjsonb_idx USING spgist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp && t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #@> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #@> t2.temp')
WHERE op = '#@> tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp <@# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp <@# t2.temp')
WHERE op = '<@# tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp && t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #@> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #@> t2.t')
WHERE op = '#@> tstzspan';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp <@# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp <@# t2.t')
WHERE op = '<@# tstzspan';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t && t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t ~= t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t #@> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t #@> t1.temp')
WHERE op = 'tstzspan #@> tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t <@# t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t <@# t1.temp')
WHERE op = 'tstzspan <@# tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t -|- t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t <<# t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tjsonb';
UPDATE test_tjsonb_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t #>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tjsonb';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tjsonb_idx_quadtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tjsonb_idx_kdtree ON tbl_tjsonb_idx USING spgist(temp tjsonb_kdtree_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp && t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #@> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #@> t2.temp')
WHERE op = '#@> tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp <@# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp <@# t2.temp')
WHERE op = '<@# tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp && t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #@> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #@> t2.t')
WHERE op = '#@> tstzspan';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp <@# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp <@# t2.t')
WHERE op = '<@# tstzspan';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_idx t1, tbl_tjsonb_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t && t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t ~= t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t #@> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t #@> t1.temp')
WHERE op = 'tstzspan #@> tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t <@# t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t <@# t1.temp')
WHERE op = 'tstzspan <@# tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t -|- t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t <<# t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tjsonb';
UPDATE test_tjsonb_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t #>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tjsonb_span t2, tbl_tjsonb_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tjsonb';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tjsonb_idx_kdtree;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tjsonb_idx;
SELECT op, no_idx FROM test_tjsonb_idx ORDER BY op;
SELECT * FROM test_tjsonb_idx
WHERE no_idx IS DISTINCT FROM rtree_idx OR no_idx IS DISTINCT FROM quadtree_idx OR
  no_idx IS DISTINCT FROM kdtree_idx OR NOT rtree_probe OR NOT quadtree_probe OR
  NOT kdtree_probe
ORDER BY op;

DROP TABLE test_tjsonb_idx;
DROP TABLE tbl_tjsonb_span;
DROP TABLE tbl_tjsonb_val;
DROP TABLE tbl_tjsonb_src;
DROP TABLE tbl_tjsonb_idx;
DROP FUNCTION test_probes_index(text);

-------------------------------------------------------------------------------
