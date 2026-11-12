-------------------------------------------------------------------------------
-- The R-tree GiST, quad-tree and k-d tree SP-GiST indexes of a ts2cell, keyed
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

DROP TABLE IF EXISTS tbl_ts2cell_src;
CREATE TABLE tbl_ts2cell_src AS
SELECT k, 4.0 + (k % 20) * 0.05 AS x, 50.0 + (k / 20) * 0.05 AS y,
  timestamptz '2001-01-01' + k * interval '1 hour' AS t
FROM generate_series(1, 240) k;

-- Instants, moving points, and runs of one cell whose periods meet
DROP TABLE IF EXISTS tbl_ts2cell_idx;
CREATE TABLE tbl_ts2cell_idx AS
SELECT k, ts2cell(geoToS2Cell(geography(ST_SetSRID(ST_MakePoint(x, y), 4326)), 12), t) AS temp
FROM tbl_ts2cell_src WHERE k <= 100
UNION ALL
SELECT k, ts2cell(tgeogpoint(format('[Point(%s %s)@%s, Point(%s %s)@%s)',
    x, y, t, x + 0.04, y + 0.04, t + interval '3 hours')), 12)
FROM tbl_ts2cell_src WHERE k > 100 AND k <= 200
UNION ALL
SELECT k, ts2cell(geoToS2Cell(geography(ST_SetSRID(ST_MakePoint(4.5, 51.5), 4326)), 12),
    tstzspan(format('[%s, %s)', t, t + interval '1 hour')))
FROM tbl_ts2cell_src WHERE k > 200;
ANALYZE tbl_ts2cell_idx;

-- The box of a value, a box around it, and a box inside the moving points
DROP TABLE IF EXISTS tbl_ts2cell_box;
CREATE TABLE tbl_ts2cell_box AS
SELECT temp::stbox AS b FROM tbl_ts2cell_idx WHERE k % 4 = 0
UNION ALL
SELECT expandTime(expandSpace(temp::stbox, 0.1), interval '1 day')
FROM tbl_ts2cell_idx WHERE k % 4 = 1
UNION ALL
SELECT stbox(format('SRID=4326;GEODSTBOX XT(((%s,%s),(%s,%s)),[%s,%s])',
    x + 0.02, y + 0.02, x + 0.021, y + 0.021, t + interval '1 hour',
    t + interval '2 hours'))
FROM tbl_ts2cell_src WHERE k > 100 AND k <= 200 AND k % 4 = 2;
ANALYZE tbl_ts2cell_box;

-- A box of space alone, which the operators on time refuse: one around a
-- value, one inside a moving point, the space of a value, and one that meets it
DROP TABLE IF EXISTS tbl_ts2cell_xbox;
CREATE TABLE tbl_ts2cell_xbox AS
SELECT stbox(format('SRID=4326;GEODSTBOX X((%s,%s),(%s,%s))', x, y, x + 0.1, y + 0.1)) AS b
FROM tbl_ts2cell_src WHERE k % 4 = 3
UNION ALL
SELECT stbox(format('SRID=4326;GEODSTBOX X((%s,%s),(%s,%s))',
    x + 0.02, y + 0.02, x + 0.021, y + 0.021))
FROM tbl_ts2cell_src WHERE k > 100 AND k <= 200 AND k % 4 = 2
UNION ALL
SELECT stbox(format('SRID=4326;GEODSTBOX X((%s,%s),(%s,%s))',
    xmin(temp::stbox), ymin(temp::stbox), xmax(temp::stbox), ymax(temp::stbox)))
FROM tbl_ts2cell_idx WHERE k % 4 = 0
UNION ALL
SELECT stbox(format('SRID=4326;GEODSTBOX X((%s,%s),(%s,%s))',
    xmax(temp::stbox), ymin(temp::stbox), xmax(temp::stbox) + 0.1, ymax(temp::stbox)))
FROM tbl_ts2cell_idx WHERE k % 4 = 1;
ANALYZE tbl_ts2cell_xbox;

-- The period of a value, one around it, one inside it, and one that meets it
DROP TABLE IF EXISTS tbl_ts2cell_span;
CREATE TABLE tbl_ts2cell_span AS
SELECT temp::tstzspan AS t FROM tbl_ts2cell_idx WHERE k % 4 = 0
UNION ALL
SELECT tstzspan(format('[%s, %s]', t - interval '1 day', t + interval '1 day'))
FROM tbl_ts2cell_src WHERE k % 4 = 1
UNION ALL
SELECT tstzspan(format('[%s, %s]', t + interval '1 hour', t + interval '2 hours'))
FROM tbl_ts2cell_src WHERE k > 100 AND k <= 200 AND k % 4 = 2
UNION ALL
SELECT tstzspan(format('[%s, %s)', t - interval '1 hour', t))
FROM tbl_ts2cell_src WHERE k % 4 = 3;
ANALYZE tbl_ts2cell_span;

DROP TABLE IF EXISTS test_ts2cell_idx;
CREATE TABLE test_ts2cell_idx(op text, no_idx text, rtree_idx text, quadtree_idx text,
  kdtree_idx text, rtree_probe boolean, quadtree_probe boolean,
  kdtree_probe boolean);

INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<< ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp << t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&< ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &< t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&& ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&> ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &> t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('>> ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp >> t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('~= ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('@> ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp @> t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<@ ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&<| ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<| t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<<| ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<| t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('|>> ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |>> t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('|&> ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |&> t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('-|- ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&<# ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<<# ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('#>> ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('#&> ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<< stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&< stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&& stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&> stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('>> stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('~= stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('@> stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<@ stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&<| stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<<| stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('|>> stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('|&> stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('-|- stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&<# stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<# t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<<# stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<# t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('#>> stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #>> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('#&> stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #&> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&& tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp && t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('~= tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('@> tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp @> t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<@ tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <@ t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('-|- tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp -|- t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&<# tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<<# tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('#>> tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('#&> tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<< stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&< stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&& stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&> stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('>> stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('~= stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('@> stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<@ stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&<| stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<<| stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('|>> stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('|&> stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('-|- stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp -|- t2.b)::text);

-- With no index a plan cannot probe one, whatever the planner settings
SET enable_seqscan = off;
SELECT test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b') AS no_index_probe;
RESET enable_seqscan;

-------------------------------------------------------------------------------

CREATE INDEX tbl_ts2cell_idx_rtree ON tbl_ts2cell_idx USING gist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp << t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp << t2.temp')
WHERE op = '<< ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &< t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp && t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp && t2.temp')
WHERE op = '&& ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp >> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp @> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<# t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<# t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp && t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp ~= t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp @> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <@ t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp -|- t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp &<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #>> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #&> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_ts2cell_idx_rtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_ts2cell_idx_quadtree ON tbl_ts2cell_idx USING spgist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp << t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp << t2.temp')
WHERE op = '<< ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp && t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp && t2.temp')
WHERE op = '&& ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp && t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp @> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_ts2cell_idx_quadtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_ts2cell_idx_kdtree ON tbl_ts2cell_idx USING spgist(temp ts2cell_kdtree_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp << t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp << t2.temp')
WHERE op = '<< ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp && t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp && t2.temp')
WHERE op = '&& ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp && t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp @> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_ts2cell_idx_kdtree;

-------------------------------------------------------------------------------

SELECT op, split_part(no_idx, ':', 1) AS no_idx FROM test_ts2cell_idx ORDER BY op;
SELECT * FROM test_ts2cell_idx
WHERE no_idx IS DISTINCT FROM rtree_idx OR no_idx IS DISTINCT FROM quadtree_idx OR
  no_idx IS DISTINCT FROM kdtree_idx OR NOT rtree_probe OR NOT quadtree_probe OR
  NOT kdtree_probe
ORDER BY op;

DROP TABLE test_ts2cell_idx;
DROP TABLE tbl_ts2cell_span;
DROP TABLE tbl_ts2cell_xbox;
DROP TABLE tbl_ts2cell_box;
-------------------------------------------------------------------------------
-- A column of stbox values indexed by GiST, quad-tree and k-d tree answers the
-- queries of a ts2cell as the scan does: <<, &<, &&, &>, >>, ~=, @>, <@, &<|, <<|, |>>, |&>, -|-, &<#, <<#, #>>, #&>
-------------------------------------------------------------------------------

CREATE TABLE tbl_stbox_ts2cell AS
SELECT k, temp::stbox AS b FROM tbl_ts2cell_idx WHERE temp IS NOT NULL;
CREATE TABLE tbl_ts2cell_q AS
SELECT temp AS q FROM tbl_ts2cell_idx WHERE temp IS NOT NULL ORDER BY k LIMIT 3;
CREATE FUNCTION stbox_ts2cell_answers() RETURNS text AS $$
  SELECT string_agg(coalesce(c::text, 'null'), ',' ORDER BY o) FROM (
    SELECT 1, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b << q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 2, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b &< q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 3, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b && q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 4, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b &> q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 5, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b >> q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 6, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b ~= q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 7, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b @> q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 8, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b <@ q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 9, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b &<| q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 10, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b <<| q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 11, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b |>> q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 12, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b |&> q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 13, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b -|- q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 14, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b &<# q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 15, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b <<# q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 16, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b #>> q) FROM tbl_ts2cell_q
    UNION ALL
    SELECT 17, (SELECT count(*) FROM tbl_stbox_ts2cell WHERE b #&> q) FROM tbl_ts2cell_q) t(o, c);
$$ LANGUAGE SQL;
CREATE FUNCTION stbox_ts2cell_probes_index(query text)
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
CREATE FUNCTION stbox_ts2cell_indexed() RETURNS text AS $$
  SELECT string_agg(CASE WHEN stbox_ts2cell_probes_index(query) THEN 't' ELSE 'f' END, '' ORDER BY o)
  FROM (VALUES
    (1, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b << (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (2, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b &< (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (3, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b && (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (4, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b &> (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (5, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b >> (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (6, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b ~= (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (7, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b @> (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (8, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b <@ (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (9, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b &<| (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (10, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b <<| (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (11, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b |>> (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (12, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b |&> (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (13, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b -|- (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (14, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b &<# (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (15, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b <<# (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (16, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b #>> (SELECT q FROM tbl_ts2cell_q LIMIT 1)'),
    (17, 'SELECT count(*) FROM tbl_stbox_ts2cell WHERE b #&> (SELECT q FROM tbl_ts2cell_q LIMIT 1)')) t(o, query);
$$ LANGUAGE SQL;
CREATE TABLE tbl_stbox_ts2cell_seq AS SELECT stbox_ts2cell_answers() AS a;
SELECT a FROM tbl_stbox_ts2cell_seq;
SET enable_seqscan = off;
CREATE INDEX tbl_stbox_ts2cell_gist_idx ON tbl_stbox_ts2cell USING gist(b);
SELECT stbox_ts2cell_indexed(), stbox_ts2cell_answers() = a
FROM tbl_stbox_ts2cell_seq;
DROP INDEX tbl_stbox_ts2cell_gist_idx;
CREATE INDEX tbl_stbox_ts2cell_quad_idx ON tbl_stbox_ts2cell USING spgist(b);
SELECT stbox_ts2cell_indexed(), stbox_ts2cell_answers() = a
FROM tbl_stbox_ts2cell_seq;
DROP INDEX tbl_stbox_ts2cell_quad_idx;
CREATE INDEX tbl_stbox_ts2cell_kd_idx ON tbl_stbox_ts2cell
  USING spgist(b stbox_kdtree_ops);
SELECT stbox_ts2cell_indexed(), stbox_ts2cell_answers() = a
FROM tbl_stbox_ts2cell_seq;
RESET enable_seqscan;
DROP TABLE tbl_stbox_ts2cell, tbl_ts2cell_q, tbl_stbox_ts2cell_seq;
DROP FUNCTION stbox_ts2cell_indexed(), stbox_ts2cell_answers(), stbox_ts2cell_probes_index(text);

DROP TABLE tbl_ts2cell_idx;
DROP TABLE tbl_ts2cell_src;
DROP FUNCTION test_probes_index(text);

-------------------------------------------------------------------------------
