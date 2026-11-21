-------------------------------------------------------------------------------
-- The R-tree GiST, quad-tree and k-d tree SP-GiST indexes of a tbigint, keyed by
-- the tbox of the value. Under each index every operator of the class, with each
-- right argument it declares, answers the rows a sequential scan gives, from a
-- plan that probes the index, and the search for the nearest values answers the
-- distances a sort gives.
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

DROP TABLE IF EXISTS test_tbigint_idx;
CREATE TEMP TABLE test_tbigint_idx(op text, no_idx text, rtree_idx text, quadtree_idx text,
  kdtree_idx text, rtree_probe boolean, quadtree_probe boolean,
  kdtree_probe boolean);

INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('<< bigintspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp << t2.v)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('<< tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('<< tbox x', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('<< tbigint', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp << t2.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&< bigintspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp &< t2.v)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&< tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&< tbox x', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&< tbigint', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &< t2.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&& tstzspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp && t2.t)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&& bigintspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp && t2.v)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&& tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&& tbox x', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&& tbigint', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&> bigintspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp &> t2.v)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&> tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&> tbox x', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&> tbigint', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &> t2.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('>> bigintspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp >> t2.v)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('>> tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('>> tbox x', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('>> tbigint', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp >> t2.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('~= tstzspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('~= bigintspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp ~= t2.v)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('~= tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('~= tbox x', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('~= tbigint', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('@> tstzspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp @> t2.t)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('@> bigintspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp @> t2.v)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('@> tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('@> tbox x', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('@> tbigint', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp @> t2.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('<@ tstzspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp <@ t2.t)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('<@ bigintspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp <@ t2.v)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('<@ tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('<@ tbox x', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('<@ tbigint', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('-|- tstzspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp -|- t2.t)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('-|- bigintspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp -|- t2.v)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('-|- tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('-|- tbox x', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('-|- tbigint', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&<# tstzspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&<# tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &<# t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('&<# tbigint', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('<<# tstzspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('<<# tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp <<# t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('<<# tbigint', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('#>> tstzspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('#>> tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp #>> t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('#>> tbigint', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('#&> tstzspan', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('#&> tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp #&> t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('#&> tbigint', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('?&& tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('%&& tbox', (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tbox >> tbigint', (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b >> t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tstzspan && tbigint', (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t && t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tbox && tbigint', (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b && t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tbox << tbigint', (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b << t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tstzspan ~= tbigint', (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t ~= t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tbox ~= tbigint', (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b ~= t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tstzspan <@ tbigint', (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t <@ t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tbox <@ tbigint', (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b <@ t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tstzspan @> tbigint', (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t @> t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tbox @> tbigint', (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b @> t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tstzspan -|- tbigint', (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t -|- t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tbox -|- tbigint', (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b -|- t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tstzspan #>> tbigint', (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t #>> t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tbox #>> tbigint', (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b #>> t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tstzspan <<# tbigint', (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t <<# t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('tbox <<# tbigint', (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b <<# t1.temp)::text);
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('|=| bigintspan', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tbigint_vspan t2, LATERAL (SELECT t1.temp |=| t2.v AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.v LIMIT 3) s WHERE t2.knn));
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('|=| tbox', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tbigint_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn));
INSERT INTO test_tbigint_idx(op, no_idx) VALUES ('|=| tbigint', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tbigint_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn));

-- With no index a plan cannot probe one, whatever the planner settings
SET enable_seqscan = off;
SELECT pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp << t2.b') AS no_index_probe;
RESET enable_seqscan;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbigint_idx_rtree ON tbl_tbigint_idx USING gist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp << t2.v)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp << t2.v')
WHERE op = '<< bigintspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tbox x';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp << t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp &< t2.v)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp &< t2.v')
WHERE op = '&< bigintspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tbox x';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &< t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp && t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp && t2.v)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp && t2.v')
WHERE op = '&& bigintspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tbox x';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp && t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp &> t2.v)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp &> t2.v')
WHERE op = '&> bigintspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tbox x';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &> t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp >> t2.v)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp >> t2.v')
WHERE op = '>> bigintspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tbox x';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp >> t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp ~= t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp ~= t2.v)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp ~= t2.v')
WHERE op = '~= bigintspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tbox x';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp @> t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp @> t2.v)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp @> t2.v')
WHERE op = '@> bigintspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tbox x';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp @> t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp <@ t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp <@ t2.v)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp <@ t2.v')
WHERE op = '<@ bigintspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tbox x';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp -|- t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp -|- t2.v)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp -|- t2.v')
WHERE op = '-|- bigintspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tbox x';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp &<# t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &<# t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp <<# t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp <<# t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp #>> t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp #>> t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp #&> t2.t)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp #&> t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b >> t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b >> t1.temp')
WHERE op = 'tbox >> tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t && t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b && t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'tbox && tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b << t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b << t1.temp')
WHERE op = 'tbox << tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t ~= t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b ~= t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'tbox ~= tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t <@ t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b <@ t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'tbox <@ tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t @> t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t @> t1.temp')
WHERE op = 'tstzspan @> tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b @> t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'tbox @> tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t -|- t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b -|- t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = 'tbox -|- tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t #>> t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b #>> t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'tbox #>> tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t <<# t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b <<# t1.temp)::text,
  rtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'tbox <<# tbigint';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tbigint_vspan t2, LATERAL (SELECT t1.temp |=| t2.v AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.v LIMIT 3) s WHERE t2.knn),
  rtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tbigint_vspan t2, LATERAL (SELECT t1.temp |=| t2.v AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.v LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| bigintspan';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tbigint_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  rtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tbigint_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| tbox';
UPDATE test_tbigint_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tbigint_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  rtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tbigint_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| tbigint';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbigint_idx_rtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbigint_idx_quadtree ON tbl_tbigint_idx USING spgist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp << t2.v)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp << t2.v')
WHERE op = '<< bigintspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tbox x';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp << t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp &< t2.v)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp &< t2.v')
WHERE op = '&< bigintspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tbox x';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp && t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp && t2.v)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp && t2.v')
WHERE op = '&& bigintspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tbox x';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp && t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp &> t2.v)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp &> t2.v')
WHERE op = '&> bigintspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tbox x';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp >> t2.v)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp >> t2.v')
WHERE op = '>> bigintspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tbox x';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp ~= t2.v)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp ~= t2.v')
WHERE op = '~= bigintspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tbox x';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp @> t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp @> t2.v)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp @> t2.v')
WHERE op = '@> bigintspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tbox x';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp <@ t2.v)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp <@ t2.v')
WHERE op = '<@ bigintspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tbox x';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp -|- t2.v)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp -|- t2.v')
WHERE op = '-|- bigintspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tbox x';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b >> t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b >> t1.temp')
WHERE op = 'tbox >> tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t && t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b && t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'tbox && tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b << t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b << t1.temp')
WHERE op = 'tbox << tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t ~= t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b ~= t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'tbox ~= tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t <@ t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b <@ t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'tbox <@ tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t @> t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t @> t1.temp')
WHERE op = 'tstzspan @> tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b @> t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'tbox @> tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t -|- t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b -|- t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = 'tbox -|- tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t #>> t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b #>> t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'tbox #>> tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t <<# t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b <<# t1.temp)::text,
  quadtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'tbox <<# tbigint';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tbigint_vspan t2, LATERAL (SELECT t1.temp |=| t2.v AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.v LIMIT 3) s WHERE t2.knn),
  quadtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tbigint_vspan t2, LATERAL (SELECT t1.temp |=| t2.v AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.v LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| bigintspan';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tbigint_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  quadtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tbigint_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| tbox';
UPDATE test_tbigint_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tbigint_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  quadtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tbigint_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| tbigint';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbigint_idx_quadtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbigint_idx_kdtree ON tbl_tbigint_idx USING spgist(temp tbigint_kdtree_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp << t2.v)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp << t2.v')
WHERE op = '<< bigintspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tbox x';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp << t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp &< t2.v)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp &< t2.v')
WHERE op = '&< bigintspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tbox x';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp && t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp && t2.v)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp && t2.v')
WHERE op = '&& bigintspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tbox x';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp && t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp &> t2.v)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp &> t2.v')
WHERE op = '&> bigintspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tbox x';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp >> t2.v)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp >> t2.v')
WHERE op = '>> bigintspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tbox x';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp ~= t2.v)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp ~= t2.v')
WHERE op = '~= bigintspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tbox x';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp @> t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp @> t2.v)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp @> t2.v')
WHERE op = '@> bigintspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tbox x';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp <@ t2.v)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp <@ t2.v')
WHERE op = '<@ bigintspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tbox x';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp -|- t2.v)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_vspan t2 WHERE t1.temp -|- t2.v')
WHERE op = '-|- bigintspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tbox x';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_idx t1, tbl_tbigint_box t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b >> t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b >> t1.temp')
WHERE op = 'tbox >> tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t && t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b && t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'tbox && tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b << t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b << t1.temp')
WHERE op = 'tbox << tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t ~= t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b ~= t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'tbox ~= tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t <@ t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b <@ t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'tbox <@ tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t @> t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t @> t1.temp')
WHERE op = 'tstzspan @> tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b @> t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'tbox @> tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t -|- t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b -|- t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = 'tbox -|- tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t #>> t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b #>> t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'tbox #>> tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t <<# t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_span t2, tbl_tbigint_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b <<# t1.temp)::text,
  kdtree_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_box t2, tbl_tbigint_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'tbox <<# tbigint';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tbigint_vspan t2, LATERAL (SELECT t1.temp |=| t2.v AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.v LIMIT 3) s WHERE t2.knn),
  kdtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tbigint_vspan t2, LATERAL (SELECT t1.temp |=| t2.v AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.v LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| bigintspan';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tbigint_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  kdtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tbigint_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| tbox';
UPDATE test_tbigint_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tbigint_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  kdtree_probe = pg_temp.test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tbigint_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tbigint_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| tbigint';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbigint_idx_kdtree;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbigint_idx;
SELECT COUNT(*) FROM test_tbigint_idx;
SELECT op, no_idx FROM test_tbigint_idx WHERE op NOT LIKE '%|=|%' ORDER BY op;
-- The distances of the nearest values are compared, not stated
SELECT op FROM test_tbigint_idx WHERE op LIKE '%|=|%' AND no_idx IS NULL ORDER BY op;
SELECT * FROM test_tbigint_idx
WHERE no_idx IS DISTINCT FROM rtree_idx OR no_idx IS DISTINCT FROM quadtree_idx OR
  no_idx IS DISTINCT FROM kdtree_idx OR NOT rtree_probe OR NOT quadtree_probe OR
  NOT kdtree_probe
ORDER BY op;

DROP TABLE test_tbigint_idx;
DROP TABLE tbl_tbigint_span;
DROP TABLE tbl_tbigint_vspan;
DROP TABLE tbl_tbigint_xbox;
DROP TABLE tbl_tbigint_box;
DROP TABLE tbl_tbigint_val;
DROP TABLE tbl_tbigint_src;
DROP TABLE tbl_tbigint_idx;
-- Every row carries the same value, so a split of the quad-tree finds keys that
-- are all equal
CREATE TEMP TABLE tbl_tbigint_allthesame AS
SELECT k, tbigint(bigint '5', t) AS temp
FROM tbl_tstzspan_big;
CREATE INDEX tbl_tbigint_allthesame_quadtree ON tbl_tbigint_allthesame USING spgist(temp);
SELECT COUNT(*) AS rows_scan FROM tbl_tbigint_allthesame WHERE temp && tbox 'TBOXBIGINT X([5, 5])';
SET enable_seqscan = off;
SELECT pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbigint_allthesame WHERE temp && tbox ''TBOXBIGINT X([5, 5])''') AS probe;
SELECT COUNT(*) AS rows_index FROM tbl_tbigint_allthesame WHERE temp && tbox 'TBOXBIGINT X([5, 5])';
RESET enable_seqscan;
DROP TABLE tbl_tbigint_allthesame;

DROP FUNCTION pg_temp.test_probes_index(text);

-------------------------------------------------------------------------------

