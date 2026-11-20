-------------------------------------------------------------------------------
-- The R-tree GiST, quad-tree and k-d tree SP-GiST indexes of a tgeography in three
-- dimensions, keyed by the stbox of the value. Under each index every operator of
-- the class, with each right argument it declares, answers the rows a sequential
-- scan gives, from a plan that probes the index, and the search for the nearest
-- values answers the distances a sort gives.
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

DROP TABLE IF EXISTS tbl_tgeography3d_idx;
CREATE TEMP TABLE tbl_tgeography3d_idx AS
WITH base(k, temp) AS (
  SELECT k, temp FROM tbl_tgeography3d_big
  UNION ALL
  SELECT k + 10100, temp FROM tbl_tgeography3d_inst
  UNION ALL
  SELECT k + 10200, temp FROM tbl_tgeography3d_discseq
  UNION ALL
  SELECT k + 10300, temp FROM tbl_tgeography3d_contseq
  UNION ALL
  SELECT k + 10400, temp FROM tbl_tgeography3d_seqset )
SELECT k + n * 10500 AS k, shiftTime(temp, n * interval '1 week') AS temp
FROM base, generate_series(0, 0) AS n;
ANALYZE tbl_tgeography3d_idx;

-- The values, boxes and periods the operators are given. Those of the fixture
-- are random, so they meet few values exactly: the others come from the rows of
-- the table, so that every operator has rows to answer.
DROP TABLE IF EXISTS tbl_tgeography3d_src;
CREATE TEMP TABLE tbl_tgeography3d_src AS
SELECT k, temp, temp::stbox AS b, temp::tstzspan AS t
FROM tbl_tgeography3d_idx WHERE k % 1000 = 0;

-- A value of the fixture, a value of the table, and one whose period ends where
-- that of the value starts. The search for the nearest values, whose distance
-- costs more than a box, is given the second alone.
DROP TABLE IF EXISTS tbl_tgeography3d_val;
CREATE TEMP TABLE tbl_tgeography3d_val AS
SELECT temp, false AS knn FROM tbl_tgeography3d WHERE k % 20 = 0
UNION ALL
SELECT temp, true FROM tbl_tgeography3d_src
UNION ALL
SELECT shiftTime(temp, lower(t) - upper(t)), false FROM tbl_tgeography3d_src;
ANALYZE tbl_tgeography3d_val;

-- The box of a value of the fixture, the box of a value, one around it, one
-- inside it, and one whose period meets it
DROP TABLE IF EXISTS tbl_tgeography3d_box;
CREATE TEMP TABLE tbl_tgeography3d_box AS
SELECT temp::stbox AS b, false AS knn FROM tbl_tgeography3d WHERE k % 20 = 0
UNION ALL
SELECT b, true FROM tbl_tgeography3d_src
UNION ALL
SELECT expandTime(expandSpace(b, 0.1), interval '1 day'), false FROM tbl_tgeography3d_src
UNION ALL
SELECT stbox(format('SRID=7844;GEODSTBOX ZT(((%s,%s,%s),(%s,%s,%s)),[%s,%s])',
    (xmin(b) + xmax(b)) / 2, (ymin(b) + ymax(b)) / 2, (zmin(b) + zmax(b)) / 2, (xmin(b) + xmax(b)) / 2 + 0.000001, (ymin(b) + ymax(b)) / 2 + 0.000001, (zmin(b) + zmax(b)) / 2 + 0.000001,
    lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2)), false
FROM tbl_tgeography3d_src
UNION ALL
SELECT stbox(format('SRID=7844;GEODSTBOX ZT(((%s,%s,%s),(%s,%s,%s)),[%s,%s%s)',
    xmin(b), ymin(b), zmin(b), xmax(b), ymax(b), zmax(b), lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END)), false
FROM tbl_tgeography3d_src;
ANALYZE tbl_tgeography3d_box;

-- A box of space alone, which the operators on time refuse: one around a value,
-- one inside it, the space of a value, and one that meets it
DROP TABLE IF EXISTS tbl_tgeography3d_xbox;
CREATE TEMP TABLE tbl_tgeography3d_xbox AS
SELECT stbox(format('SRID=7844;GEODSTBOX Z((%s,%s,%s),(%s,%s,%s))',
    xmin(b) - 0.1, ymin(b) - 0.1, zmin(b) - 0.1, xmax(b) + 0.1, ymax(b) + 0.1, zmax(b) + 0.1)) AS b
FROM tbl_tgeography3d_src
UNION ALL
SELECT stbox(format('SRID=7844;GEODSTBOX Z((%s,%s,%s),(%s,%s,%s))',
    (xmin(b) + xmax(b)) / 2, (ymin(b) + ymax(b)) / 2, (zmin(b) + zmax(b)) / 2, (xmin(b) + xmax(b)) / 2 + 0.000001, (ymin(b) + ymax(b)) / 2 + 0.000001, (zmin(b) + zmax(b)) / 2 + 0.000001)) AS b
FROM tbl_tgeography3d_src
UNION ALL
SELECT stbox(format('SRID=7844;GEODSTBOX Z((%s,%s,%s),(%s,%s,%s))',
    xmin(b), ymin(b), zmin(b), xmax(b), ymax(b), zmax(b))) AS b
FROM tbl_tgeography3d_src
UNION ALL
SELECT stbox(format('SRID=7844;GEODSTBOX Z((%s,%s,%s),(%s,%s,%s))',
    xmax(b), ymin(b), zmin(b), xmax(b) + 0.1, ymax(b), zmax(b))) AS b
FROM tbl_tgeography3d_src;
ANALYZE tbl_tgeography3d_xbox;

-- A period of the fixture, the period of a value, one around it, one inside it,
-- and one that meets it
DROP TABLE IF EXISTS tbl_tgeography3d_span;
CREATE TEMP TABLE tbl_tgeography3d_span AS
SELECT t FROM tbl_tstzspan WHERE k % 20 = 0
UNION ALL
SELECT t FROM tbl_tgeography3d_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) - interval '1 day', upper(t) + interval '1 day'))
FROM tbl_tgeography3d_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2))
FROM tbl_tgeography3d_src
UNION ALL
SELECT tstzspan(format('[%s, %s%s', lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END))
FROM tbl_tgeography3d_src;
ANALYZE tbl_tgeography3d_span;

-- A point at a corner of the box of a value
DROP TABLE IF EXISTS tbl_tgeography3d_geo;
CREATE TEMP TABLE tbl_tgeography3d_geo AS
SELECT ST_SetSRID(ST_MakePoint(xmin(b), ymin(b), zmin(b)), 7844) AS g, true AS knn FROM tbl_tgeography3d_src;
ANALYZE tbl_tgeography3d_geo;

DROP TABLE IF EXISTS test_tgeography_idx;
CREATE TEMP TABLE test_tgeography_idx(op text, no_idx text, rtree_idx text, quadtree_idx text,
  kdtree_idx text, rtree_probe boolean, quadtree_probe boolean,
  kdtree_probe boolean);

INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('&& tstzspan', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp && t2.t)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('&& stbox', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('&& stbox x', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('&& tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('~= tstzspan', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('~= stbox', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('~= stbox x', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('~= tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('@> tstzspan', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp @> t2.t)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('@> stbox', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('@> stbox x', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('@> tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp @> t2.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('<@ tstzspan', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp <@ t2.t)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('<@ stbox', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('<@ stbox x', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('<@ tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('-|- tstzspan', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp -|- t2.t)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('-|- stbox', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('-|- stbox x', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('-|- tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('&<# tstzspan', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('&<# stbox', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp &<# t2.b)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('&<# tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('<<# tstzspan', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('<<# stbox', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp <<# t2.b)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('<<# tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('#>> tstzspan', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('#>> stbox', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp #>> t2.b)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('#>> tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('#&> tstzspan', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('#&> stbox', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp #&> t2.b)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('#&> tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('tstzspan && tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t && t1.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('stbox && tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b && t1.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('tstzspan ~= tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t ~= t1.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('stbox ~= tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b ~= t1.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('tstzspan <@ tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t <@ t1.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('stbox <@ tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b <@ t1.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('tstzspan @> tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t @> t1.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('stbox @> tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b @> t1.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('tstzspan -|- tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t -|- t1.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('stbox -|- tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b -|- t1.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('tstzspan #>> tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t #>> t1.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('stbox #>> tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b #>> t1.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('tstzspan <<# tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t <<# t1.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('stbox <<# tgeography', (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b <<# t1.temp)::text);
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('|=| stbox', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tgeography3d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn));
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('|=| tgeography', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tgeography3d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn));
INSERT INTO test_tgeography_idx(op, no_idx) VALUES ('|=| geography', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tgeography3d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn));

-- With no index a plan cannot probe one, whatever the planner settings
SET enable_seqscan = off;
SELECT pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp && t2.b') AS no_index_probe;
RESET enable_seqscan;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeography3d_idx_rtree ON tbl_tgeography3d_idx USING gist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp && t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp && t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp ~= t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp @> t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp @> t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp <@ t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp -|- t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp &<# t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp &<# t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp <<# t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp <<# t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp #>> t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp #>> t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp #&> t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp #&> t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t && t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b && t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'stbox && tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t ~= t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b ~= t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'stbox ~= tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t <@ t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b <@ t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'stbox <@ tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t @> t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t @> t1.temp')
WHERE op = 'tstzspan @> tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b @> t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'stbox @> tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t -|- t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b -|- t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = 'stbox -|- tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t #>> t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b #>> t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'stbox #>> tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t <<# t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b <<# t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'stbox <<# tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tgeography3d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  rtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tgeography3d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| stbox';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tgeography3d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  rtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tgeography3d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| tgeography';
UPDATE test_tgeography_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tgeography3d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn),
  rtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tgeography3d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| geography';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tgeography3d_idx_rtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeography3d_idx_quadtree ON tbl_tgeography3d_idx USING spgist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp && t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp && t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp @> t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t && t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b && t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'stbox && tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t ~= t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b ~= t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'stbox ~= tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t <@ t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b <@ t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'stbox <@ tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t @> t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t @> t1.temp')
WHERE op = 'tstzspan @> tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b @> t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'stbox @> tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t -|- t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b -|- t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = 'stbox -|- tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t #>> t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b #>> t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'stbox #>> tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t <<# t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b <<# t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'stbox <<# tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tgeography3d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  quadtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tgeography3d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| stbox';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tgeography3d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  quadtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tgeography3d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| tgeography';
UPDATE test_tgeography_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tgeography3d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn),
  quadtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tgeography3d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| geography';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tgeography3d_idx_quadtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeography3d_idx_kdtree ON tbl_tgeography3d_idx USING spgist(temp tgeography_kdtree_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp && t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp && t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp @> t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_idx t1, tbl_tgeography3d_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t && t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b && t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'stbox && tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t ~= t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b ~= t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'stbox ~= tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t <@ t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b <@ t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'stbox <@ tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t @> t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t @> t1.temp')
WHERE op = 'tstzspan @> tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b @> t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'stbox @> tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t -|- t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b -|- t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = 'stbox -|- tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t #>> t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b #>> t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'stbox #>> tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t <<# t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_span t2, tbl_tgeography3d_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b <<# t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeography3d_box t2, tbl_tgeography3d_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'stbox <<# tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tgeography3d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  kdtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tgeography3d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| stbox';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tgeography3d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  kdtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tgeography3d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| tgeography';
UPDATE test_tgeography_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tgeography3d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn),
  kdtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tgeography3d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tgeography3d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| geography';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tgeography3d_idx_kdtree;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeography3d_idx;
SELECT COUNT(*) FROM test_tgeography_idx;
SELECT op, no_idx FROM test_tgeography_idx WHERE op NOT LIKE '%|=|%' ORDER BY op;
-- The distances of the nearest values are compared, not stated
SELECT op FROM test_tgeography_idx WHERE op LIKE '%|=|%' AND no_idx IS NULL ORDER BY op;
SELECT * FROM test_tgeography_idx
WHERE no_idx IS DISTINCT FROM rtree_idx OR no_idx IS DISTINCT FROM quadtree_idx OR
  no_idx IS DISTINCT FROM kdtree_idx OR NOT rtree_probe OR NOT quadtree_probe OR
  NOT kdtree_probe
ORDER BY op;

DROP TABLE test_tgeography_idx;
DROP TABLE tbl_tgeography3d_geo;
DROP TABLE tbl_tgeography3d_span;
DROP TABLE tbl_tgeography3d_xbox;
DROP TABLE tbl_tgeography3d_box;
DROP TABLE tbl_tgeography3d_val;
DROP TABLE tbl_tgeography3d_src;
DROP TABLE tbl_tgeography3d_idx;


DROP FUNCTION pg_temp.test_probes_index(text);

-------------------------------------------------------------------------------

