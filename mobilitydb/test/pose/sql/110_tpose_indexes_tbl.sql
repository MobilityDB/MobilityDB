-------------------------------------------------------------------------------
-- The R-tree GiST, quad-tree and k-d tree SP-GiST indexes of a tpose, keyed by the
-- stbox of the value, in two and in three dimensions. Under each index every
-- operator of the class, with each right argument it declares, answers the rows a
-- sequential scan gives, from a plan that probes the index, and the search for the
-- nearest values answers the distances a sort gives. The operators on Z take a
-- value in three dimensions alone.
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

DROP TABLE IF EXISTS tbl_tpose2d_idx;
CREATE TABLE tbl_tpose2d_idx AS
WITH base(k, temp) AS (
  SELECT k, temp FROM tbl_tpose2d
  UNION ALL
  SELECT k + 100, inst FROM tbl_tpose2d_inst
  UNION ALL
  SELECT k + 200, seq FROM tbl_tpose2d_discseq
  UNION ALL
  SELECT k + 300, seq FROM tbl_tpose2d_seq
  UNION ALL
  SELECT k + 400, ss FROM tbl_tpose2d_seqset )
SELECT k + n * 500 AS k, shiftTime(temp, n * interval '1 week') AS temp
FROM base, generate_series(0, 9) AS n;
ANALYZE tbl_tpose2d_idx;

-- The values, boxes and periods the operators are given. Those of the fixture
-- are random, so they meet few values exactly: the others come from the rows of
-- the table, so that every operator has rows to answer.
DROP TABLE IF EXISTS tbl_tpose2d_src;
CREATE TABLE tbl_tpose2d_src AS
SELECT k, temp, temp::stbox AS b, temp::tstzspan AS t
FROM tbl_tpose2d_idx WHERE k % 470 = 0;

-- A value of the fixture, a value of the table, and one whose period ends where
-- that of the value starts. The search for the nearest values, whose distance
-- costs more than a box, is given the second alone.
DROP TABLE IF EXISTS tbl_tpose2d_val;
CREATE TABLE tbl_tpose2d_val AS
SELECT temp, false AS knn FROM tbl_tpose2d WHERE k % 20 = 0
UNION ALL
SELECT temp, true FROM tbl_tpose2d_src
UNION ALL
SELECT shiftTime(temp, lower(t) - upper(t)), false FROM tbl_tpose2d_src;
ANALYZE tbl_tpose2d_val;

-- The box of a value of the fixture, the box of a value, one around it, one
-- inside it, and one whose period meets it
DROP TABLE IF EXISTS tbl_tpose2d_box;
CREATE TABLE tbl_tpose2d_box AS
SELECT temp::stbox AS b, false AS knn FROM tbl_tpose2d WHERE k % 20 = 0
UNION ALL
SELECT b, true FROM tbl_tpose2d_src
UNION ALL
SELECT expandTime(expandSpace(b, 0.1), interval '1 day'), false FROM tbl_tpose2d_src
UNION ALL
SELECT stbox(format('SRID=3812;STBOX XT(((%s,%s),(%s,%s)),[%s,%s])',
    (xmin(b) + xmax(b)) / 2, (ymin(b) + ymax(b)) / 2, (xmin(b) + xmax(b)) / 2 + 0.000001, (ymin(b) + ymax(b)) / 2 + 0.000001,
    lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2)), false
FROM tbl_tpose2d_src
UNION ALL
SELECT stbox(format('SRID=3812;STBOX XT(((%s,%s),(%s,%s)),[%s,%s%s)',
    xmin(b), ymin(b), xmax(b), ymax(b), lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END)), false
FROM tbl_tpose2d_src;
ANALYZE tbl_tpose2d_box;

-- A box of space alone, which the operators on time refuse: one around a value,
-- one inside it, the space of a value, and one that meets it
DROP TABLE IF EXISTS tbl_tpose2d_xbox;
CREATE TABLE tbl_tpose2d_xbox AS
SELECT stbox(format('SRID=3812;STBOX X((%s,%s),(%s,%s))',
    xmin(b) - 0.1, ymin(b) - 0.1, xmax(b) + 0.1, ymax(b) + 0.1)) AS b
FROM tbl_tpose2d_src
UNION ALL
SELECT stbox(format('SRID=3812;STBOX X((%s,%s),(%s,%s))',
    (xmin(b) + xmax(b)) / 2, (ymin(b) + ymax(b)) / 2, (xmin(b) + xmax(b)) / 2 + 0.000001, (ymin(b) + ymax(b)) / 2 + 0.000001)) AS b
FROM tbl_tpose2d_src
UNION ALL
SELECT stbox(format('SRID=3812;STBOX X((%s,%s),(%s,%s))',
    xmin(b), ymin(b), xmax(b), ymax(b))) AS b
FROM tbl_tpose2d_src
UNION ALL
SELECT stbox(format('SRID=3812;STBOX X((%s,%s),(%s,%s))',
    xmax(b), ymin(b), xmax(b) + 0.1, ymax(b))) AS b
FROM tbl_tpose2d_src;
ANALYZE tbl_tpose2d_xbox;

-- A period of the fixture, the period of a value, one around it, one inside it,
-- and one that meets it
DROP TABLE IF EXISTS tbl_tpose2d_span;
CREATE TABLE tbl_tpose2d_span AS
SELECT t FROM tbl_tstzspan WHERE k % 20 = 0
UNION ALL
SELECT t FROM tbl_tpose2d_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) - interval '1 day', upper(t) + interval '1 day'))
FROM tbl_tpose2d_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2))
FROM tbl_tpose2d_src
UNION ALL
SELECT tstzspan(format('[%s, %s%s', lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END))
FROM tbl_tpose2d_src;
ANALYZE tbl_tpose2d_span;

-- A point at a corner of the box of a value
DROP TABLE IF EXISTS tbl_tpose2d_geo;
CREATE TABLE tbl_tpose2d_geo AS
SELECT ST_SetSRID(ST_MakePoint(xmin(b), ymin(b)), 3812) AS g, true AS knn FROM tbl_tpose2d_src;
ANALYZE tbl_tpose2d_geo;

DROP TABLE IF EXISTS tbl_tpose3d_idx;
CREATE TABLE tbl_tpose3d_idx AS
WITH base(k, temp) AS (
  SELECT k, temp FROM tbl_tpose3d
  UNION ALL
  SELECT k + 100, inst FROM tbl_tpose3d_inst
  UNION ALL
  SELECT k + 200, seq FROM tbl_tpose3d_discseq
  UNION ALL
  SELECT k + 300, seq FROM tbl_tpose3d_seq
  UNION ALL
  SELECT k + 400, ss FROM tbl_tpose3d_seqset )
SELECT k + n * 500 AS k, shiftTime(temp, n * interval '1 week') AS temp
FROM base, generate_series(0, 9) AS n;
ANALYZE tbl_tpose3d_idx;

-- The values, boxes and periods the operators are given. Those of the fixture
-- are random, so they meet few values exactly: the others come from the rows of
-- the table, so that every operator has rows to answer.
DROP TABLE IF EXISTS tbl_tpose3d_src;
CREATE TABLE tbl_tpose3d_src AS
SELECT k, temp, temp::stbox AS b, temp::tstzspan AS t
FROM tbl_tpose3d_idx WHERE k % 470 = 0;

-- A value of the fixture, a value of the table, and one whose period ends where
-- that of the value starts. The search for the nearest values, whose distance
-- costs more than a box, is given the second alone.
DROP TABLE IF EXISTS tbl_tpose3d_val;
CREATE TABLE tbl_tpose3d_val AS
SELECT temp, false AS knn FROM tbl_tpose3d WHERE k % 20 = 0
UNION ALL
SELECT temp, true FROM tbl_tpose3d_src
UNION ALL
SELECT shiftTime(temp, lower(t) - upper(t)), false FROM tbl_tpose3d_src;
ANALYZE tbl_tpose3d_val;

-- The box of a value of the fixture, the box of a value, one around it, one
-- inside it, and one whose period meets it
DROP TABLE IF EXISTS tbl_tpose3d_box;
CREATE TABLE tbl_tpose3d_box AS
SELECT temp::stbox AS b, false AS knn FROM tbl_tpose3d WHERE k % 20 = 0
UNION ALL
SELECT b, true FROM tbl_tpose3d_src
UNION ALL
SELECT expandTime(expandSpace(b, 0.1), interval '1 day'), false FROM tbl_tpose3d_src
UNION ALL
SELECT stbox(format('SRID=3812;STBOX ZT(((%s,%s,%s),(%s,%s,%s)),[%s,%s])',
    (xmin(b) + xmax(b)) / 2, (ymin(b) + ymax(b)) / 2, (zmin(b) + zmax(b)) / 2, (xmin(b) + xmax(b)) / 2 + 0.000001, (ymin(b) + ymax(b)) / 2 + 0.000001, (zmin(b) + zmax(b)) / 2 + 0.000001,
    lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2)), false
FROM tbl_tpose3d_src
UNION ALL
SELECT stbox(format('SRID=3812;STBOX ZT(((%s,%s,%s),(%s,%s,%s)),[%s,%s%s)',
    xmin(b), ymin(b), zmin(b), xmax(b), ymax(b), zmax(b), lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END)), false
FROM tbl_tpose3d_src;
ANALYZE tbl_tpose3d_box;

-- A box of space alone, which the operators on time refuse: one around a value,
-- one inside it, the space of a value, and one that meets it
DROP TABLE IF EXISTS tbl_tpose3d_xbox;
CREATE TABLE tbl_tpose3d_xbox AS
SELECT stbox(format('SRID=3812;STBOX Z((%s,%s,%s),(%s,%s,%s))',
    xmin(b) - 0.1, ymin(b) - 0.1, zmin(b) - 0.1, xmax(b) + 0.1, ymax(b) + 0.1, zmax(b) + 0.1)) AS b
FROM tbl_tpose3d_src
UNION ALL
SELECT stbox(format('SRID=3812;STBOX Z((%s,%s,%s),(%s,%s,%s))',
    (xmin(b) + xmax(b)) / 2, (ymin(b) + ymax(b)) / 2, (zmin(b) + zmax(b)) / 2, (xmin(b) + xmax(b)) / 2 + 0.000001, (ymin(b) + ymax(b)) / 2 + 0.000001, (zmin(b) + zmax(b)) / 2 + 0.000001)) AS b
FROM tbl_tpose3d_src
UNION ALL
SELECT stbox(format('SRID=3812;STBOX Z((%s,%s,%s),(%s,%s,%s))',
    xmin(b), ymin(b), zmin(b), xmax(b), ymax(b), zmax(b))) AS b
FROM tbl_tpose3d_src
UNION ALL
SELECT stbox(format('SRID=3812;STBOX Z((%s,%s,%s),(%s,%s,%s))',
    xmax(b), ymin(b), zmin(b), xmax(b) + 0.1, ymax(b), zmax(b))) AS b
FROM tbl_tpose3d_src;
ANALYZE tbl_tpose3d_xbox;

-- A period of the fixture, the period of a value, one around it, one inside it,
-- and one that meets it
DROP TABLE IF EXISTS tbl_tpose3d_span;
CREATE TABLE tbl_tpose3d_span AS
SELECT t FROM tbl_tstzspan WHERE k % 20 = 0
UNION ALL
SELECT t FROM tbl_tpose3d_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) - interval '1 day', upper(t) + interval '1 day'))
FROM tbl_tpose3d_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2))
FROM tbl_tpose3d_src
UNION ALL
SELECT tstzspan(format('[%s, %s%s', lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END))
FROM tbl_tpose3d_src;
ANALYZE tbl_tpose3d_span;

-- A point at a corner of the box of a value
DROP TABLE IF EXISTS tbl_tpose3d_geo;
CREATE TABLE tbl_tpose3d_geo AS
SELECT ST_SetSRID(ST_MakePoint(xmin(b), ymin(b), zmin(b)), 3812) AS g, true AS knn FROM tbl_tpose3d_src;
ANALYZE tbl_tpose3d_geo;

DROP TABLE IF EXISTS test_tpose_idx;
CREATE TABLE test_tpose_idx(op text, no_idx text, rtree_idx text, quadtree_idx text,
  kdtree_idx text, rtree_probe boolean, quadtree_probe boolean,
  kdtree_probe boolean);

INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d << stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d << stbox x', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d << tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp << t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d &< stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d &< stbox x', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d &< tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &< t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d && tstzspan', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp && t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d && stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d && stbox x', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d && tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d &> stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d &> stbox x', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d &> tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d >> stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d >> stbox x', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d >> tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp >> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d ~= tstzspan', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d ~= stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d ~= stbox x', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d ~= tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d @> tstzspan', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp @> t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d @> stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d @> stbox x', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d @> tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp @> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d <@ tstzspan', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp <@ t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d <@ stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d <@ stbox x', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d <@ tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d &<| stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d &<| stbox x', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d &<| tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &<| t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d <<| stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d <<| stbox x', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d <<| tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <<| t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d |>> stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d |>> stbox x', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d |>> tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp |>> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d |&> stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d |&> stbox x', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d |&> tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp |&> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d -|- tstzspan', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp -|- t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d -|- stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d -|- stbox x', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d -|- tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d &<# tstzspan', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d &<# stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &<# t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d &<# tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d <<# tstzspan', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d <<# stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <<# t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d <<# tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d #>> tstzspan', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d #>> stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp #>> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d #>> tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d #&> tstzspan', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d #&> stbox', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp #&> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d #&> tpose', (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d stbox >> tpose', (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b >> t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d tstzspan && tpose', (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t && t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d stbox && tpose', (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b && t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d stbox << tpose', (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b << t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d tstzspan ~= tpose', (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t ~= t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d stbox ~= tpose', (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b ~= t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d tstzspan <@ tpose', (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t <@ t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d stbox <@ tpose', (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <@ t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d tstzspan @> tpose', (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t @> t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d stbox @> tpose', (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b @> t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d stbox |>> tpose', (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b |>> t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d stbox <<| tpose', (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <<| t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d tstzspan -|- tpose', (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t -|- t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d stbox -|- tpose', (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b -|- t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d tstzspan #>> tpose', (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t #>> t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d stbox #>> tpose', (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b #>> t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d tstzspan <<# tpose', (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t <<# t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d stbox <<# tpose', (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <<# t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d |=| stbox', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose2d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn));
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d |=| tpose', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose2d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn));
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('2d |=| geometry', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose2d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn));
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d << stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d << stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d << tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp << t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &< stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &< stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &< tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &< t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d && tstzspan', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp && t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d && stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d && stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d && tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &> stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &> stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d >> stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d >> stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d >> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp >> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d ~= tstzspan', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d ~= stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d ~= stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d ~= tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d @> tstzspan', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp @> t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d @> stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d @> stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d @> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp @> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d <@ tstzspan', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp <@ t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d <@ stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d <@ stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d <@ tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &<| stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &<| stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &<| tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &<| t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d <<| stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d <<| stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d <<| tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <<| t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d |>> stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d |>> stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d |>> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp |>> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d |&> stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d |&> stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d |&> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp |&> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d -|- tstzspan', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp -|- t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d -|- stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d -|- stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d -|- tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &<# tstzspan', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &<# stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &<# t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &<# tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d <<# tstzspan', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d <<# stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <<# t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d <<# tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d #>> tstzspan', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d #>> stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp #>> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d #>> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d #&> tstzspan', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d #&> stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp #&> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d #&> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &</ stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &</ t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &</ stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &</ t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d &</ tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &</ t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d <</ stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <</ t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d <</ stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <</ t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d <</ tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <</ t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d />> stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp />> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d />> stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp />> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d />> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp />> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d /&> stbox', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp /&> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d /&> stbox x', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp /&> t2.b)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d /&> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp /&> t2.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d stbox >> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b >> t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d tstzspan && tpose', (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t && t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d stbox && tpose', (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b && t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d stbox << tpose', (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b << t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d tstzspan ~= tpose', (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t ~= t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d stbox ~= tpose', (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b ~= t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d tstzspan <@ tpose', (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t <@ t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d stbox <@ tpose', (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <@ t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d tstzspan @> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t @> t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d stbox @> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b @> t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d stbox |>> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b |>> t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d stbox <<| tpose', (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <<| t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d tstzspan -|- tpose', (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t -|- t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d stbox -|- tpose', (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b -|- t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d tstzspan #>> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t #>> t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d stbox #>> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b #>> t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d tstzspan <<# tpose', (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t <<# t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d stbox <<# tpose', (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <<# t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d stbox />> tpose', (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b />> t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d stbox <</ tpose', (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <</ t1.temp)::text);
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d |=| stbox', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose3d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn));
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d |=| tpose', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose3d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn));
INSERT INTO test_tpose_idx(op, no_idx) VALUES ('3d |=| geometry', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose3d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn));

-- With no index a plan cannot probe one, whatever the planner settings
SET enable_seqscan = off;
SELECT test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp << t2.b') AS no_index_probe;
RESET enable_seqscan;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpose2d_idx_rtree ON tbl_tpose2d_idx USING gist(temp);
CREATE INDEX tbl_tpose3d_idx_rtree ON tbl_tpose3d_idx USING gist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp << t2.b')
WHERE op = '2d << stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '2d << stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp << t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp << t2.temp')
WHERE op = '2d << tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &< t2.b')
WHERE op = '2d &< stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '2d &< stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &< t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '2d &< tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp && t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp && t2.t')
WHERE op = '2d && tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp && t2.b')
WHERE op = '2d && stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '2d && stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp && t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp && t2.temp')
WHERE op = '2d && tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &> t2.b')
WHERE op = '2d &> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '2d &> stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '2d &> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp >> t2.b')
WHERE op = '2d >> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '2d >> stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp >> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '2d >> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp ~= t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '2d ~= tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '2d ~= stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '2d ~= stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '2d ~= tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp @> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp @> t2.t')
WHERE op = '2d @> tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp @> t2.b')
WHERE op = '2d @> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '2d @> stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp @> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '2d @> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp <@ t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '2d <@ tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '2d <@ stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '2d <@ stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '2d <@ tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '2d &<| stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '2d &<| stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '2d &<| tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '2d <<| stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '2d <<| stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '2d <<| tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp |>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '2d |>> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '2d |>> stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '2d |>> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp |&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '2d |&> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '2d |&> stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '2d |&> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp -|- t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '2d -|- tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '2d -|- stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '2d -|- stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '2d -|- tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp &<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '2d &<# tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &<# t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '2d &<# stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '2d &<# tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp <<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '2d <<# tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <<# t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '2d <<# stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '2d <<# tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp #>> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '2d #>> tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp #>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '2d #>> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '2d #>> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp #&> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '2d #&> tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp #&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '2d #&> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '2d #&> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b >> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b >> t1.temp')
WHERE op = '2d stbox >> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t && t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t && t1.temp')
WHERE op = '2d tstzspan && tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b && t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b && t1.temp')
WHERE op = '2d stbox && tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b << t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b << t1.temp')
WHERE op = '2d stbox << tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t ~= t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = '2d tstzspan ~= tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b ~= t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = '2d stbox ~= tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t <@ t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = '2d tstzspan <@ tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <@ t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = '2d stbox <@ tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t @> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t @> t1.temp')
WHERE op = '2d tstzspan @> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b @> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b @> t1.temp')
WHERE op = '2d stbox @> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b |>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = '2d stbox |>> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <<| t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = '2d stbox <<| tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t -|- t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = '2d tstzspan -|- tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b -|- t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = '2d stbox -|- tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t #>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = '2d tstzspan #>> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b #>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = '2d stbox #>> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t <<# t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = '2d tstzspan <<# tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <<# t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = '2d stbox <<# tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose2d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  rtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose2d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '2d |=| stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose2d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  rtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose2d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '2d |=| tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose2d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn),
  rtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose2d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn')
WHERE op = '2d |=| geometry';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp << t2.b')
WHERE op = '3d << stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '3d << stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp << t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp << t2.temp')
WHERE op = '3d << tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &< t2.b')
WHERE op = '3d &< stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '3d &< stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &< t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '3d &< tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp && t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp && t2.t')
WHERE op = '3d && tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp && t2.b')
WHERE op = '3d && stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '3d && stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp && t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp && t2.temp')
WHERE op = '3d && tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &> t2.b')
WHERE op = '3d &> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '3d &> stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '3d &> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp >> t2.b')
WHERE op = '3d >> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '3d >> stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp >> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '3d >> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp ~= t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '3d ~= tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '3d ~= stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '3d ~= stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '3d ~= tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp @> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp @> t2.t')
WHERE op = '3d @> tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp @> t2.b')
WHERE op = '3d @> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '3d @> stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp @> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '3d @> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp <@ t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '3d <@ tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '3d <@ stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '3d <@ stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '3d <@ tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '3d &<| stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '3d &<| stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '3d &<| tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '3d <<| stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '3d <<| stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '3d <<| tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp |>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '3d |>> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '3d |>> stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '3d |>> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp |&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '3d |&> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '3d |&> stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '3d |&> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp -|- t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '3d -|- tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '3d -|- stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '3d -|- stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '3d -|- tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp &<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '3d &<# tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &<# t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '3d &<# stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '3d &<# tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp <<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '3d <<# tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <<# t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '3d <<# stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '3d <<# tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp #>> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '3d #>> tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp #>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '3d #>> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '3d #>> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp #&> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '3d #&> tstzspan';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp #&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '3d #&> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '3d #&> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &</ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &</ t2.b')
WHERE op = '3d &</ stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &</ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '3d &</ stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &</ t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &</ t2.temp')
WHERE op = '3d &</ tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <</ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <</ t2.b')
WHERE op = '3d <</ stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <</ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '3d <</ stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <</ t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <</ t2.temp')
WHERE op = '3d <</ tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp />> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp />> t2.b')
WHERE op = '3d />> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp />> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp />> t2.b')
WHERE op = '3d />> stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp />> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp />> t2.temp')
WHERE op = '3d />> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp /&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp /&> t2.b')
WHERE op = '3d /&> stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp /&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '3d /&> stbox x';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp /&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp /&> t2.temp')
WHERE op = '3d /&> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b >> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b >> t1.temp')
WHERE op = '3d stbox >> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t && t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t && t1.temp')
WHERE op = '3d tstzspan && tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b && t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b && t1.temp')
WHERE op = '3d stbox && tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b << t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b << t1.temp')
WHERE op = '3d stbox << tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t ~= t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = '3d tstzspan ~= tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b ~= t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = '3d stbox ~= tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t <@ t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = '3d tstzspan <@ tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <@ t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = '3d stbox <@ tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t @> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t @> t1.temp')
WHERE op = '3d tstzspan @> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b @> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b @> t1.temp')
WHERE op = '3d stbox @> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b |>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = '3d stbox |>> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <<| t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = '3d stbox <<| tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t -|- t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = '3d tstzspan -|- tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b -|- t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = '3d stbox -|- tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t #>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = '3d tstzspan #>> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b #>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = '3d stbox #>> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t <<# t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = '3d tstzspan <<# tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <<# t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = '3d stbox <<# tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b />> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b />> t1.temp')
WHERE op = '3d stbox />> tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <</ t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <</ t1.temp')
WHERE op = '3d stbox <</ tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose3d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  rtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose3d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '3d |=| stbox';
UPDATE test_tpose_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose3d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  rtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose3d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '3d |=| tpose';
UPDATE test_tpose_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose3d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn),
  rtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose3d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn')
WHERE op = '3d |=| geometry';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpose2d_idx_rtree;
DROP INDEX tbl_tpose3d_idx_rtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpose2d_idx_quadtree ON tbl_tpose2d_idx USING spgist(temp);
CREATE INDEX tbl_tpose3d_idx_quadtree ON tbl_tpose3d_idx USING spgist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp << t2.b')
WHERE op = '2d << stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '2d << stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp << t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp << t2.temp')
WHERE op = '2d << tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &< t2.b')
WHERE op = '2d &< stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '2d &< stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '2d &< tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp && t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp && t2.t')
WHERE op = '2d && tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp && t2.b')
WHERE op = '2d && stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '2d && stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp && t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp && t2.temp')
WHERE op = '2d && tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &> t2.b')
WHERE op = '2d &> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '2d &> stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '2d &> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp >> t2.b')
WHERE op = '2d >> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '2d >> stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '2d >> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '2d ~= tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '2d ~= stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '2d ~= stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '2d ~= tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp @> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp @> t2.t')
WHERE op = '2d @> tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp @> t2.b')
WHERE op = '2d @> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '2d @> stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '2d @> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '2d <@ tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '2d <@ stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '2d <@ stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '2d <@ tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '2d &<| stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '2d &<| stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '2d &<| tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '2d <<| stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '2d <<| stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '2d <<| tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '2d |>> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '2d |>> stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '2d |>> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '2d |&> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '2d |&> stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '2d |&> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '2d -|- tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '2d -|- stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '2d -|- stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '2d -|- tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '2d &<# tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '2d &<# stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '2d &<# tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '2d <<# tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '2d <<# stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '2d <<# tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '2d #>> tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '2d #>> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '2d #>> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '2d #&> tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '2d #&> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '2d #&> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b >> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b >> t1.temp')
WHERE op = '2d stbox >> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t && t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t && t1.temp')
WHERE op = '2d tstzspan && tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b && t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b && t1.temp')
WHERE op = '2d stbox && tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b << t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b << t1.temp')
WHERE op = '2d stbox << tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t ~= t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = '2d tstzspan ~= tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b ~= t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = '2d stbox ~= tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t <@ t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = '2d tstzspan <@ tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <@ t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = '2d stbox <@ tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t @> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t @> t1.temp')
WHERE op = '2d tstzspan @> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b @> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b @> t1.temp')
WHERE op = '2d stbox @> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b |>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = '2d stbox |>> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <<| t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = '2d stbox <<| tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t -|- t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = '2d tstzspan -|- tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b -|- t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = '2d stbox -|- tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t #>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = '2d tstzspan #>> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b #>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = '2d stbox #>> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t <<# t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = '2d tstzspan <<# tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <<# t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = '2d stbox <<# tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose2d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  quadtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose2d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '2d |=| stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose2d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  quadtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose2d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '2d |=| tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose2d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn),
  quadtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose2d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn')
WHERE op = '2d |=| geometry';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp << t2.b')
WHERE op = '3d << stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '3d << stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp << t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp << t2.temp')
WHERE op = '3d << tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &< t2.b')
WHERE op = '3d &< stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '3d &< stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '3d &< tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp && t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp && t2.t')
WHERE op = '3d && tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp && t2.b')
WHERE op = '3d && stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '3d && stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp && t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp && t2.temp')
WHERE op = '3d && tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &> t2.b')
WHERE op = '3d &> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '3d &> stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '3d &> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp >> t2.b')
WHERE op = '3d >> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '3d >> stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '3d >> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '3d ~= tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '3d ~= stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '3d ~= stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '3d ~= tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp @> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp @> t2.t')
WHERE op = '3d @> tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp @> t2.b')
WHERE op = '3d @> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '3d @> stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '3d @> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '3d <@ tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '3d <@ stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '3d <@ stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '3d <@ tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '3d &<| stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '3d &<| stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '3d &<| tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '3d <<| stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '3d <<| stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '3d <<| tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '3d |>> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '3d |>> stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '3d |>> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '3d |&> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '3d |&> stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '3d |&> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '3d -|- tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '3d -|- stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '3d -|- stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '3d -|- tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '3d &<# tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '3d &<# stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '3d &<# tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '3d <<# tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '3d <<# stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '3d <<# tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '3d #>> tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '3d #>> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '3d #>> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '3d #&> tstzspan';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '3d #&> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '3d #&> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &</ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &</ t2.b')
WHERE op = '3d &</ stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &</ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '3d &</ stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &</ t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &</ t2.temp')
WHERE op = '3d &</ tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <</ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <</ t2.b')
WHERE op = '3d <</ stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <</ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '3d <</ stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <</ t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <</ t2.temp')
WHERE op = '3d <</ tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp />> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp />> t2.b')
WHERE op = '3d />> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp />> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp />> t2.b')
WHERE op = '3d />> stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp />> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp />> t2.temp')
WHERE op = '3d />> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp /&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp /&> t2.b')
WHERE op = '3d /&> stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp /&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '3d /&> stbox x';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp /&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp /&> t2.temp')
WHERE op = '3d /&> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b >> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b >> t1.temp')
WHERE op = '3d stbox >> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t && t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t && t1.temp')
WHERE op = '3d tstzspan && tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b && t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b && t1.temp')
WHERE op = '3d stbox && tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b << t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b << t1.temp')
WHERE op = '3d stbox << tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t ~= t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = '3d tstzspan ~= tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b ~= t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = '3d stbox ~= tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t <@ t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = '3d tstzspan <@ tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <@ t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = '3d stbox <@ tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t @> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t @> t1.temp')
WHERE op = '3d tstzspan @> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b @> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b @> t1.temp')
WHERE op = '3d stbox @> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b |>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = '3d stbox |>> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <<| t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = '3d stbox <<| tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t -|- t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = '3d tstzspan -|- tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b -|- t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = '3d stbox -|- tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t #>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = '3d tstzspan #>> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b #>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = '3d stbox #>> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t <<# t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = '3d tstzspan <<# tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <<# t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = '3d stbox <<# tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b />> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b />> t1.temp')
WHERE op = '3d stbox />> tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <</ t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <</ t1.temp')
WHERE op = '3d stbox <</ tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose3d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  quadtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose3d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '3d |=| stbox';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose3d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  quadtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose3d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '3d |=| tpose';
UPDATE test_tpose_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose3d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn),
  quadtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose3d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn')
WHERE op = '3d |=| geometry';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpose2d_idx_quadtree;
DROP INDEX tbl_tpose3d_idx_quadtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpose2d_idx_kdtree ON tbl_tpose2d_idx USING spgist(temp tpose_kdtree_ops);
CREATE INDEX tbl_tpose3d_idx_kdtree ON tbl_tpose3d_idx USING spgist(temp tpose_kdtree_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp << t2.b')
WHERE op = '2d << stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '2d << stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp << t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp << t2.temp')
WHERE op = '2d << tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &< t2.b')
WHERE op = '2d &< stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '2d &< stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '2d &< tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp && t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp && t2.t')
WHERE op = '2d && tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp && t2.b')
WHERE op = '2d && stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '2d && stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp && t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp && t2.temp')
WHERE op = '2d && tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &> t2.b')
WHERE op = '2d &> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '2d &> stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '2d &> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp >> t2.b')
WHERE op = '2d >> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '2d >> stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '2d >> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '2d ~= tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '2d ~= stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '2d ~= stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '2d ~= tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp @> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp @> t2.t')
WHERE op = '2d @> tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp @> t2.b')
WHERE op = '2d @> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '2d @> stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '2d @> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '2d <@ tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '2d <@ stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '2d <@ stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '2d <@ tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '2d &<| stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '2d &<| stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '2d &<| tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '2d <<| stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '2d <<| stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '2d <<| tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '2d |>> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '2d |>> stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '2d |>> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '2d |&> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '2d |&> stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '2d |&> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '2d -|- tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '2d -|- stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '2d -|- stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '2d -|- tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '2d &<# tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '2d &<# stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '2d &<# tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '2d <<# tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '2d <<# stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '2d <<# tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '2d #>> tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '2d #>> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '2d #>> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '2d #&> tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '2d #&> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_idx t1, tbl_tpose2d_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '2d #&> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b >> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b >> t1.temp')
WHERE op = '2d stbox >> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t && t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t && t1.temp')
WHERE op = '2d tstzspan && tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b && t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b && t1.temp')
WHERE op = '2d stbox && tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b << t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b << t1.temp')
WHERE op = '2d stbox << tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t ~= t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = '2d tstzspan ~= tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b ~= t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = '2d stbox ~= tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t <@ t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = '2d tstzspan <@ tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <@ t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = '2d stbox <@ tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t @> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t @> t1.temp')
WHERE op = '2d tstzspan @> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b @> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b @> t1.temp')
WHERE op = '2d stbox @> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b |>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = '2d stbox |>> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <<| t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = '2d stbox <<| tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t -|- t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = '2d tstzspan -|- tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b -|- t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = '2d stbox -|- tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t #>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = '2d tstzspan #>> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b #>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = '2d stbox #>> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t <<# t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_span t2, tbl_tpose2d_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = '2d tstzspan <<# tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <<# t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose2d_box t2, tbl_tpose2d_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = '2d stbox <<# tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose2d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  kdtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose2d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '2d |=| stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose2d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  kdtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose2d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '2d |=| tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose2d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn),
  kdtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose2d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tpose2d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn')
WHERE op = '2d |=| geometry';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp << t2.b')
WHERE op = '3d << stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '3d << stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp << t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp << t2.temp')
WHERE op = '3d << tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &< t2.b')
WHERE op = '3d &< stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '3d &< stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '3d &< tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp && t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp && t2.t')
WHERE op = '3d && tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp && t2.b')
WHERE op = '3d && stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '3d && stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp && t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp && t2.temp')
WHERE op = '3d && tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &> t2.b')
WHERE op = '3d &> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '3d &> stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '3d &> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp >> t2.b')
WHERE op = '3d >> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '3d >> stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '3d >> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '3d ~= tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '3d ~= stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '3d ~= stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '3d ~= tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp @> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp @> t2.t')
WHERE op = '3d @> tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp @> t2.b')
WHERE op = '3d @> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '3d @> stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '3d @> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '3d <@ tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '3d <@ stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '3d <@ stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '3d <@ tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '3d &<| stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '3d &<| stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '3d &<| tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '3d <<| stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '3d <<| stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '3d <<| tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '3d |>> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '3d |>> stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '3d |>> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '3d |&> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '3d |&> stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '3d |&> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '3d -|- tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '3d -|- stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '3d -|- stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '3d -|- tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '3d &<# tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '3d &<# stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '3d &<# tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '3d <<# tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '3d <<# stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '3d <<# tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '3d #>> tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '3d #>> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '3d #>> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '3d #&> tstzspan';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '3d #&> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '3d #&> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &</ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp &</ t2.b')
WHERE op = '3d &</ stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &</ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '3d &</ stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &</ t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp &</ t2.temp')
WHERE op = '3d &</ tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <</ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp <</ t2.b')
WHERE op = '3d <</ stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <</ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '3d <</ stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <</ t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp <</ t2.temp')
WHERE op = '3d <</ tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp />> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp />> t2.b')
WHERE op = '3d />> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp />> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp />> t2.b')
WHERE op = '3d />> stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp />> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp />> t2.temp')
WHERE op = '3d />> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp /&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_box t2 WHERE t1.temp /&> t2.b')
WHERE op = '3d /&> stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp /&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_xbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '3d /&> stbox x';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp /&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_idx t1, tbl_tpose3d_val t2 WHERE t1.temp /&> t2.temp')
WHERE op = '3d /&> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b >> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b >> t1.temp')
WHERE op = '3d stbox >> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t && t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t && t1.temp')
WHERE op = '3d tstzspan && tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b && t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b && t1.temp')
WHERE op = '3d stbox && tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b << t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b << t1.temp')
WHERE op = '3d stbox << tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t ~= t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = '3d tstzspan ~= tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b ~= t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = '3d stbox ~= tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t <@ t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = '3d tstzspan <@ tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <@ t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = '3d stbox <@ tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t @> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t @> t1.temp')
WHERE op = '3d tstzspan @> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b @> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b @> t1.temp')
WHERE op = '3d stbox @> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b |>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = '3d stbox |>> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <<| t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = '3d stbox <<| tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t -|- t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = '3d tstzspan -|- tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b -|- t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = '3d stbox -|- tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t #>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = '3d tstzspan #>> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b #>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = '3d stbox #>> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t <<# t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_span t2, tbl_tpose3d_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = '3d tstzspan <<# tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <<# t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = '3d stbox <<# tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b />> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b />> t1.temp')
WHERE op = '3d stbox />> tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <</ t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tpose3d_box t2, tbl_tpose3d_idx t1 WHERE t2.b <</ t1.temp')
WHERE op = '3d stbox <</ tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose3d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  kdtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose3d_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '3d |=| stbox';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose3d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  kdtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose3d_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '3d |=| tpose';
UPDATE test_tpose_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tpose3d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn),
  kdtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tpose3d_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tpose3d_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn')
WHERE op = '3d |=| geometry';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpose2d_idx_kdtree;
DROP INDEX tbl_tpose3d_idx_kdtree;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpose2d_idx;
SELECT COUNT(*) FROM tbl_tpose3d_idx;
SELECT COUNT(*) FROM test_tpose_idx;
SELECT op, no_idx FROM test_tpose_idx WHERE op NOT LIKE '%|=|%' ORDER BY op;
-- The distances of the nearest values are compared, not stated
SELECT op FROM test_tpose_idx WHERE op LIKE '%|=|%' AND no_idx IS NULL ORDER BY op;
SELECT * FROM test_tpose_idx
WHERE no_idx IS DISTINCT FROM rtree_idx OR no_idx IS DISTINCT FROM quadtree_idx OR
  no_idx IS DISTINCT FROM kdtree_idx OR NOT rtree_probe OR NOT quadtree_probe OR
  NOT kdtree_probe
ORDER BY op;

DROP TABLE test_tpose_idx;
DROP TABLE tbl_tpose2d_geo;
DROP TABLE tbl_tpose2d_span;
DROP TABLE tbl_tpose2d_xbox;
DROP TABLE tbl_tpose2d_box;
DROP TABLE tbl_tpose2d_val;
DROP TABLE tbl_tpose2d_src;
DROP TABLE tbl_tpose2d_idx;
DROP TABLE tbl_tpose3d_geo;
DROP TABLE tbl_tpose3d_span;
DROP TABLE tbl_tpose3d_xbox;
DROP TABLE tbl_tpose3d_box;
DROP TABLE tbl_tpose3d_val;
DROP TABLE tbl_tpose3d_src;
DROP TABLE tbl_tpose3d_idx;
DROP FUNCTION test_probes_index(text);

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- A column of stbox values indexed by GiST, quad-tree and k-d tree answers the
-- queries of a tpose as the scan does: <<, &<, &&, &>, >>, ~=, @>, <@, &<|, <<|, |>>, |&>, -|-, &<#, <<#, #>>, #&>, &</, <</, />>, /&>, |=|
-------------------------------------------------------------------------------

CREATE TABLE tbl_stbox_tpose AS
SELECT k, temp::stbox AS b FROM tbl_tpose3d WHERE temp IS NOT NULL;
CREATE TABLE tbl_tpose_q AS
SELECT temp AS q FROM tbl_tpose3d WHERE temp IS NOT NULL ORDER BY k LIMIT 3;
CREATE FUNCTION stbox_tpose_answers() RETURNS text AS $$
  SELECT string_agg(coalesce(c::text, 'null'), ',' ORDER BY o) FROM (
    SELECT 1, (SELECT count(*) FROM tbl_stbox_tpose WHERE b << q) FROM tbl_tpose_q
    UNION ALL
    SELECT 2, (SELECT count(*) FROM tbl_stbox_tpose WHERE b &< q) FROM tbl_tpose_q
    UNION ALL
    SELECT 3, (SELECT count(*) FROM tbl_stbox_tpose WHERE b && q) FROM tbl_tpose_q
    UNION ALL
    SELECT 4, (SELECT count(*) FROM tbl_stbox_tpose WHERE b &> q) FROM tbl_tpose_q
    UNION ALL
    SELECT 5, (SELECT count(*) FROM tbl_stbox_tpose WHERE b >> q) FROM tbl_tpose_q
    UNION ALL
    SELECT 6, (SELECT count(*) FROM tbl_stbox_tpose WHERE b ~= q) FROM tbl_tpose_q
    UNION ALL
    SELECT 7, (SELECT count(*) FROM tbl_stbox_tpose WHERE b @> q) FROM tbl_tpose_q
    UNION ALL
    SELECT 8, (SELECT count(*) FROM tbl_stbox_tpose WHERE b <@ q) FROM tbl_tpose_q
    UNION ALL
    SELECT 9, (SELECT count(*) FROM tbl_stbox_tpose WHERE b &<| q) FROM tbl_tpose_q
    UNION ALL
    SELECT 10, (SELECT count(*) FROM tbl_stbox_tpose WHERE b <<| q) FROM tbl_tpose_q
    UNION ALL
    SELECT 11, (SELECT count(*) FROM tbl_stbox_tpose WHERE b |>> q) FROM tbl_tpose_q
    UNION ALL
    SELECT 12, (SELECT count(*) FROM tbl_stbox_tpose WHERE b |&> q) FROM tbl_tpose_q
    UNION ALL
    SELECT 13, (SELECT count(*) FROM tbl_stbox_tpose WHERE b -|- q) FROM tbl_tpose_q
    UNION ALL
    SELECT 14, (SELECT count(*) FROM tbl_stbox_tpose WHERE b &<# q) FROM tbl_tpose_q
    UNION ALL
    SELECT 15, (SELECT count(*) FROM tbl_stbox_tpose WHERE b <<# q) FROM tbl_tpose_q
    UNION ALL
    SELECT 16, (SELECT count(*) FROM tbl_stbox_tpose WHERE b #>> q) FROM tbl_tpose_q
    UNION ALL
    SELECT 17, (SELECT count(*) FROM tbl_stbox_tpose WHERE b #&> q) FROM tbl_tpose_q
    UNION ALL
    SELECT 18, (SELECT count(*) FROM tbl_stbox_tpose WHERE b &</ q) FROM tbl_tpose_q
    UNION ALL
    SELECT 19, (SELECT count(*) FROM tbl_stbox_tpose WHERE b <</ q) FROM tbl_tpose_q
    UNION ALL
    SELECT 20, (SELECT count(*) FROM tbl_stbox_tpose WHERE b />> q) FROM tbl_tpose_q
    UNION ALL
    SELECT 21, (SELECT count(*) FROM tbl_stbox_tpose WHERE b /&> q) FROM tbl_tpose_q
    UNION ALL
    SELECT 22, (SELECT sum(k) FROM (SELECT k FROM tbl_stbox_tpose
      ORDER BY b |=| q, k LIMIT 5) s) FROM tbl_tpose_q) t(o, c);
$$ LANGUAGE SQL;
CREATE FUNCTION stbox_tpose_probes_index(query text)
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
CREATE FUNCTION stbox_tpose_indexed() RETURNS text AS $$
  SELECT string_agg(CASE WHEN stbox_tpose_probes_index(query) THEN 't' ELSE 'f' END, '' ORDER BY o)
  FROM (VALUES
    (1, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b << (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (2, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b &< (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (3, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b && (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (4, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b &> (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (5, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b >> (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (6, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b ~= (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (7, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b @> (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (8, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b <@ (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (9, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b &<| (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (10, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b <<| (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (11, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b |>> (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (12, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b |&> (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (13, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b -|- (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (14, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b &<# (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (15, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b <<# (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (16, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b #>> (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (17, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b #&> (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (18, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b &</ (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (19, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b <</ (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (20, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b />> (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (21, 'SELECT count(*) FROM tbl_stbox_tpose WHERE b /&> (SELECT q FROM tbl_tpose_q LIMIT 1)'),
    (22, 'SELECT k FROM tbl_stbox_tpose ORDER BY b |=| (SELECT q FROM tbl_tpose_q LIMIT 1) LIMIT 5')) t(o, query);
$$ LANGUAGE SQL;
CREATE TABLE tbl_stbox_tpose_seq AS SELECT stbox_tpose_answers() AS a;
SELECT a FROM tbl_stbox_tpose_seq;
SET enable_seqscan = off;
CREATE INDEX tbl_stbox_tpose_gist_idx ON tbl_stbox_tpose USING gist(b);
SELECT stbox_tpose_indexed(), stbox_tpose_answers() = a
FROM tbl_stbox_tpose_seq;
DROP INDEX tbl_stbox_tpose_gist_idx;
CREATE INDEX tbl_stbox_tpose_quad_idx ON tbl_stbox_tpose USING spgist(b);
SELECT stbox_tpose_indexed(), stbox_tpose_answers() = a
FROM tbl_stbox_tpose_seq;
DROP INDEX tbl_stbox_tpose_quad_idx;
CREATE INDEX tbl_stbox_tpose_kd_idx ON tbl_stbox_tpose
  USING spgist(b stbox_kdtree_ops);
SELECT stbox_tpose_indexed(), stbox_tpose_answers() = a
FROM tbl_stbox_tpose_seq;
RESET enable_seqscan;
DROP TABLE tbl_stbox_tpose, tbl_tpose_q, tbl_stbox_tpose_seq;
DROP FUNCTION stbox_tpose_indexed(), stbox_tpose_answers(), stbox_tpose_probes_index(text);
