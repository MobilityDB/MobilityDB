-------------------------------------------------------------------------------
-- The R-tree GiST, quad-tree and k-d tree SP-GiST indexes of a th3index, keyed
-- by the stbox of the value. Under each index every operator of the class, with
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

DROP TABLE IF EXISTS tbl_th3index_idx;
CREATE TABLE tbl_th3index_idx AS SELECT k, temp FROM tbl_th3index_big;
ANALYZE tbl_th3index_idx;

-- The values, boxes and periods the operators are given. Those of the fixture
-- are random, so they meet few values exactly: the others come from the rows of
-- the table, so that every operator has rows to answer.
DROP TABLE IF EXISTS tbl_th3index_src;
CREATE TABLE tbl_th3index_src AS
SELECT k, temp, temp::stbox AS b, temp::tstzspan AS t
FROM tbl_th3index_idx WHERE k % 1000 = 0;

DROP TABLE IF EXISTS tbl_th3index_val;
CREATE TABLE tbl_th3index_val AS
SELECT temp FROM tbl_th3index WHERE k % 20 = 0
UNION ALL
SELECT temp FROM tbl_th3index_src
UNION ALL
SELECT shiftTime(temp, lower(t) - upper(t)) FROM tbl_th3index_src;
ANALYZE tbl_th3index_val;

-- A box of the fixture, the box of a value, one around it, one inside it, and
-- one whose period meets it
DROP TABLE IF EXISTS tbl_th3index_box;
CREATE TABLE tbl_th3index_box AS
SELECT b FROM tbl_geodstbox WHERE k % 20 = 0
UNION ALL
SELECT b FROM tbl_th3index_src
UNION ALL
SELECT expandTime(expandSpace(b, 0.1), interval '1 day') FROM tbl_th3index_src
UNION ALL
SELECT stbox(format('SRID=4326;GEODSTBOX XT(((%s,%s),(%s,%s)),[%s,%s])',
    (xmin(b) + xmax(b)) / 2, (ymin(b) + ymax(b)) / 2, (xmin(b) + xmax(b)) / 2 + 0.000001, (ymin(b) + ymax(b)) / 2 + 0.000001, lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2))
FROM tbl_th3index_src
UNION ALL
SELECT stbox(format('SRID=4326;GEODSTBOX XT(((%s,%s),(%s,%s)),[%s,%s%s)',
    xmin(b), ymin(b), xmax(b), ymax(b), lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END))
FROM tbl_th3index_src;
ANALYZE tbl_th3index_box;

-- A box of space alone, which the operators on time refuse: one around a value,
-- one inside it, the space of a value, and one that meets it
DROP TABLE IF EXISTS tbl_th3index_xbox;
CREATE TABLE tbl_th3index_xbox AS
SELECT stbox(format('SRID=4326;GEODSTBOX X((%s,%s),(%s,%s))',
    xmin(b) - 0.1, ymin(b) - 0.1, xmax(b) + 0.1, ymax(b) + 0.1)) AS b
FROM tbl_th3index_src
UNION ALL
SELECT stbox(format('SRID=4326;GEODSTBOX X((%s,%s),(%s,%s))',
    (xmin(b) + xmax(b)) / 2, (ymin(b) + ymax(b)) / 2, (xmin(b) + xmax(b)) / 2 + 0.000001, (ymin(b) + ymax(b)) / 2 + 0.000001))
FROM tbl_th3index_src
UNION ALL
SELECT stbox(format('SRID=4326;GEODSTBOX X((%s,%s),(%s,%s))',
    xmin(b), ymin(b), xmax(b), ymax(b)))
FROM tbl_th3index_src
UNION ALL
SELECT stbox(format('SRID=4326;GEODSTBOX X((%s,%s),(%s,%s))',
    xmax(b), ymin(b), xmax(b) + 0.1, ymax(b)))
FROM tbl_th3index_src;
ANALYZE tbl_th3index_xbox;

-- A period of the fixture, the period of a value, one around it, one inside it,
-- and one that meets it
DROP TABLE IF EXISTS tbl_th3index_span;
CREATE TABLE tbl_th3index_span AS
SELECT t FROM tbl_tstzspan WHERE k % 20 = 0
UNION ALL
SELECT t FROM tbl_th3index_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) - interval '1 day', upper(t) + interval '1 day'))
FROM tbl_th3index_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2))
FROM tbl_th3index_src
UNION ALL
SELECT tstzspan(format('[%s, %s%s', lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END))
FROM tbl_th3index_src;
ANALYZE tbl_th3index_span;

DROP TABLE IF EXISTS test_th3index_idx;
CREATE TABLE test_th3index_idx(op text, no_idx text, rtree_idx text, quadtree_idx text,
  kdtree_idx text, rtree_probe boolean, quadtree_probe boolean,
  kdtree_probe boolean);

INSERT INTO test_th3index_idx(op, no_idx) VALUES ('<< th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp << t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&< th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &< t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&& th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&> th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &> t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('>> th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp >> t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('~= th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('@> th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp @> t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('<@ th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&<| th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &<| t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('<<| th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <<| t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('|>> th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp |>> t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('|&> th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp |&> t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('-|- th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&<# th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('<<# th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('#>> th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('#&> th3index', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('<< stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&< stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&& stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('?&& stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp ?&& t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('%&& stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp %&& t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&> stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('>> stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('~= stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('@> stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('<@ stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&<| stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('<<| stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('|>> stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('|&> stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('-|- stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&<# stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &<# t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('<<# stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <<# t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('#>> stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp #>> t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('#&> stbox', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp #&> t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&& tstzspan', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp && t2.t)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('~= tstzspan', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('@> tstzspan', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp @> t2.t)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('<@ tstzspan', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp <@ t2.t)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('-|- tstzspan', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp -|- t2.t)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&<# tstzspan', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('<<# tstzspan', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('#>> tstzspan', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('#&> tstzspan', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('<< stbox x', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&< stbox x', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&& stbox x', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&> stbox x', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('>> stbox x', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('~= stbox x', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('@> stbox x', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('<@ stbox x', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('&<| stbox x', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('<<| stbox x', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('|>> stbox x', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('|&> stbox x', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('-|- stbox x', (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('stbox && th3index', (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b && t1.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('stbox @> th3index', (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b @> t1.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('stbox <@ th3index', (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <@ t1.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('stbox ~= th3index', (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b ~= t1.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('tstzspan && th3index', (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t && t1.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('tstzspan <@ th3index', (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t <@ t1.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('stbox << th3index', (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b << t1.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('stbox >> th3index', (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b >> t1.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('stbox <<| th3index', (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <<| t1.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('stbox |>> th3index', (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b |>> t1.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('tstzspan <<# th3index', (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t <<# t1.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('tstzspan #>> th3index', (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t #>> t1.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('stbox <<# th3index', (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <<# t1.temp)::text);
INSERT INTO test_th3index_idx(op, no_idx) VALUES ('stbox #>> th3index', (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b #>> t1.temp)::text);

-- With no index a plan cannot probe one, whatever the planner settings
SET enable_seqscan = off;
SELECT test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp && t2.b') AS no_index_probe;
RESET enable_seqscan;

-------------------------------------------------------------------------------

CREATE INDEX tbl_th3index_idx_rtree ON tbl_th3index_idx USING gist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp << t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp << t2.temp')
WHERE op = '<< th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &< t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp && t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp >> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp @> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp %&& t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp |>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp |&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &<# t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <<# t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp #>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp #&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp && t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp ~= t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp @> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp <@ t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp -|- t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp &<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp <<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp #>> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp #&> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b && t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'stbox && th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b @> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'stbox @> th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <@ t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'stbox <@ th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b ~= t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'stbox ~= th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t && t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t <@ t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b << t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b << t1.temp')
WHERE op = 'stbox << th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b >> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b >> t1.temp')
WHERE op = 'stbox >> th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <<| t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = 'stbox <<| th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b |>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = 'stbox |>> th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t <<# t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t #>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <<# t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'stbox <<# th3index';
UPDATE test_th3index_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b #>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'stbox #>> th3index';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_th3index_idx_rtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_th3index_idx_quadtree ON tbl_th3index_idx USING spgist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp << t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp << t2.temp')
WHERE op = '<< th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp && t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp && t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp @> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b && t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'stbox && th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b @> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'stbox @> th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <@ t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'stbox <@ th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b ~= t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'stbox ~= th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t && t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t <@ t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b << t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b << t1.temp')
WHERE op = 'stbox << th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b >> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b >> t1.temp')
WHERE op = 'stbox >> th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <<| t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = 'stbox <<| th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b |>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = 'stbox |>> th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t <<# t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t #>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <<# t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'stbox <<# th3index';
UPDATE test_th3index_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b #>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'stbox #>> th3index';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_th3index_idx_quadtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_th3index_idx_kdtree ON tbl_th3index_idx USING spgist(temp th3index_kdtree_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp << t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp << t2.temp')
WHERE op = '<< th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp && t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp && t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp @> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_idx t1, tbl_th3index_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b && t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'stbox && th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b @> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'stbox @> th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <@ t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'stbox <@ th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b ~= t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'stbox ~= th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t && t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t <@ t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b << t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b << t1.temp')
WHERE op = 'stbox << th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b >> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b >> t1.temp')
WHERE op = 'stbox >> th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <<| t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = 'stbox <<| th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b |>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = 'stbox |>> th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t <<# t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t #>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_span t2, tbl_th3index_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <<# t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'stbox <<# th3index';
UPDATE test_th3index_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b #>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_th3index_box t2, tbl_th3index_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'stbox #>> th3index';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_th3index_idx_kdtree;

-------------------------------------------------------------------------------

-- Every operator has rows to answer. The counts themselves are not stated: two
-- cells can share a box edge that each mathematical library rounds its own way.
SELECT COUNT(*) FROM test_th3index_idx;
SELECT op FROM test_th3index_idx WHERE no_idx::bigint = 0 ORDER BY op;

SELECT * FROM test_th3index_idx
WHERE no_idx IS DISTINCT FROM rtree_idx OR no_idx IS DISTINCT FROM quadtree_idx OR
  no_idx IS DISTINCT FROM kdtree_idx OR NOT rtree_probe OR NOT quadtree_probe OR
  NOT kdtree_probe
ORDER BY op;

DROP TABLE test_th3index_idx;
DROP TABLE tbl_th3index_span;
DROP TABLE tbl_th3index_xbox;
DROP TABLE tbl_th3index_box;
DROP TABLE tbl_th3index_val;
DROP TABLE tbl_th3index_src;
DROP TABLE tbl_th3index_idx;
DROP FUNCTION test_probes_index(text);

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- A column of stbox values indexed by GiST, quad-tree and k-d tree answers the
-- queries of a th3index as the scan does: <<, &<, &&, &>, >>, ~=, @>, <@, &<|, <<|, |>>, |&>, -|-, &<#, <<#, #>>, #&>
-------------------------------------------------------------------------------

CREATE TABLE tbl_stbox_th3index AS
SELECT k, temp::stbox AS b FROM tbl_th3index WHERE temp IS NOT NULL;
CREATE TABLE tbl_th3index_q AS
SELECT temp AS q FROM tbl_th3index WHERE temp IS NOT NULL ORDER BY k LIMIT 3;
CREATE FUNCTION stbox_th3index_answers() RETURNS text AS $$
  SELECT string_agg(coalesce(c::text, 'null'), ',' ORDER BY o) FROM (
    SELECT 1, (SELECT count(*) FROM tbl_stbox_th3index WHERE b << q) FROM tbl_th3index_q
    UNION ALL
    SELECT 2, (SELECT count(*) FROM tbl_stbox_th3index WHERE b &< q) FROM tbl_th3index_q
    UNION ALL
    SELECT 3, (SELECT count(*) FROM tbl_stbox_th3index WHERE b && q) FROM tbl_th3index_q
    UNION ALL
    SELECT 4, (SELECT count(*) FROM tbl_stbox_th3index WHERE b &> q) FROM tbl_th3index_q
    UNION ALL
    SELECT 5, (SELECT count(*) FROM tbl_stbox_th3index WHERE b >> q) FROM tbl_th3index_q
    UNION ALL
    SELECT 6, (SELECT count(*) FROM tbl_stbox_th3index WHERE b ~= q) FROM tbl_th3index_q
    UNION ALL
    SELECT 7, (SELECT count(*) FROM tbl_stbox_th3index WHERE b @> q) FROM tbl_th3index_q
    UNION ALL
    SELECT 8, (SELECT count(*) FROM tbl_stbox_th3index WHERE b <@ q) FROM tbl_th3index_q
    UNION ALL
    SELECT 9, (SELECT count(*) FROM tbl_stbox_th3index WHERE b &<| q) FROM tbl_th3index_q
    UNION ALL
    SELECT 10, (SELECT count(*) FROM tbl_stbox_th3index WHERE b <<| q) FROM tbl_th3index_q
    UNION ALL
    SELECT 11, (SELECT count(*) FROM tbl_stbox_th3index WHERE b |>> q) FROM tbl_th3index_q
    UNION ALL
    SELECT 12, (SELECT count(*) FROM tbl_stbox_th3index WHERE b |&> q) FROM tbl_th3index_q
    UNION ALL
    SELECT 13, (SELECT count(*) FROM tbl_stbox_th3index WHERE b -|- q) FROM tbl_th3index_q
    UNION ALL
    SELECT 14, (SELECT count(*) FROM tbl_stbox_th3index WHERE b &<# q) FROM tbl_th3index_q
    UNION ALL
    SELECT 15, (SELECT count(*) FROM tbl_stbox_th3index WHERE b <<# q) FROM tbl_th3index_q
    UNION ALL
    SELECT 16, (SELECT count(*) FROM tbl_stbox_th3index WHERE b #>> q) FROM tbl_th3index_q
    UNION ALL
    SELECT 17, (SELECT count(*) FROM tbl_stbox_th3index WHERE b #&> q) FROM tbl_th3index_q) t(o, c);
$$ LANGUAGE SQL;
CREATE FUNCTION stbox_th3index_probes_index(query text)
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
CREATE FUNCTION stbox_th3index_indexed() RETURNS text AS $$
  SELECT string_agg(CASE WHEN stbox_th3index_probes_index(query) THEN 't' ELSE 'f' END, '' ORDER BY o)
  FROM (VALUES
    (1, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b << (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (2, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b &< (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (3, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b && (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (4, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b &> (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (5, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b >> (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (6, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b ~= (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (7, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b @> (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (8, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b <@ (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (9, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b &<| (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (10, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b <<| (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (11, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b |>> (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (12, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b |&> (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (13, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b -|- (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (14, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b &<# (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (15, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b <<# (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (16, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b #>> (SELECT q FROM tbl_th3index_q LIMIT 1)'),
    (17, 'SELECT count(*) FROM tbl_stbox_th3index WHERE b #&> (SELECT q FROM tbl_th3index_q LIMIT 1)')) t(o, query);
$$ LANGUAGE SQL;
CREATE TABLE tbl_stbox_th3index_seq AS SELECT stbox_th3index_answers() AS a;
SELECT a FROM tbl_stbox_th3index_seq;
SET enable_seqscan = off;
CREATE INDEX tbl_stbox_th3index_gist_idx ON tbl_stbox_th3index USING gist(b);
SELECT stbox_th3index_indexed(), stbox_th3index_answers() = a
FROM tbl_stbox_th3index_seq;
DROP INDEX tbl_stbox_th3index_gist_idx;
CREATE INDEX tbl_stbox_th3index_quad_idx ON tbl_stbox_th3index USING spgist(b);
SELECT stbox_th3index_indexed(), stbox_th3index_answers() = a
FROM tbl_stbox_th3index_seq;
DROP INDEX tbl_stbox_th3index_quad_idx;
CREATE INDEX tbl_stbox_th3index_kd_idx ON tbl_stbox_th3index
  USING spgist(b stbox_kdtree_ops);
SELECT stbox_th3index_indexed(), stbox_th3index_answers() = a
FROM tbl_stbox_th3index_seq;
RESET enable_seqscan;
DROP TABLE tbl_stbox_th3index, tbl_th3index_q, tbl_stbox_th3index_seq;
DROP FUNCTION stbox_th3index_indexed(), stbox_th3index_answers(), stbox_th3index_probes_index(text);
