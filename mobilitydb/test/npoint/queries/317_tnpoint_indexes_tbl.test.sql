-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- The R-tree GiST, quad-tree and k-d tree SP-GiST indexes of a tnpoint, keyed by
-- the stbox of the value. Under each index every operator of the class, with each
-- right argument it declares, answers the rows a sequential scan gives, from a
-- plan that probes the index, and the search for the nearest values answers the
-- distances a sort gives.
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

DROP TABLE IF EXISTS tbl_tnpoint_idx;
CREATE TABLE tbl_tnpoint_idx AS
WITH base(k, temp) AS (
  SELECT k, temp FROM tbl_tnpoint_big
  UNION ALL
  SELECT k + 1000, inst FROM tbl_tnpoint_inst
  UNION ALL
  SELECT k + 1100, ti FROM tbl_tnpoint_discseq
  UNION ALL
  SELECT k + 1200, seq FROM tbl_tnpoint_seq
  UNION ALL
  SELECT k + 1300, ss FROM tbl_tnpoint_seqset )
SELECT k + n * 1400 AS k, shiftTime(temp, n * interval '1 week') AS temp
FROM base, generate_series(0, 0) AS n;
ANALYZE tbl_tnpoint_idx;

-- The values, boxes and periods the operators are given. Those of the fixture
-- are random, so they meet few values exactly: the others come from the rows of
-- the table, so that every operator has rows to answer.
DROP TABLE IF EXISTS tbl_tnpoint_src;
CREATE TABLE tbl_tnpoint_src AS
SELECT k, temp, temp::stbox AS b, temp::tstzspan AS t
FROM tbl_tnpoint_idx WHERE k % 200 = 0 AND temp IS NOT NULL;

-- A value of the fixture, a value of the table, and one whose period ends where
-- that of the value starts. The search for the nearest values, whose distance
-- costs more than a box, is given the second alone.
DROP TABLE IF EXISTS tbl_tnpoint_val;
CREATE TABLE tbl_tnpoint_val AS
SELECT temp, false AS knn FROM tbl_tnpoint WHERE k % 20 = 0
UNION ALL
SELECT temp, true FROM tbl_tnpoint_src
UNION ALL
SELECT shiftTime(temp, lower(t) - upper(t)), false FROM tbl_tnpoint_src;
ANALYZE tbl_tnpoint_val;

-- The box of a value of the fixture, the box of a value, one around it, one
-- inside it, and one whose period meets it
DROP TABLE IF EXISTS tbl_tnpoint_box;
CREATE TABLE tbl_tnpoint_box AS
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
CREATE TABLE tbl_tnpoint_xbox AS
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
CREATE TABLE tbl_tnpoint_span AS
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
CREATE TABLE tbl_tnpoint_geo AS
SELECT ST_SetSRID(ST_MakePoint(xmin(b), ymin(b)), 5676) AS g, true AS knn FROM tbl_tnpoint_src;
ANALYZE tbl_tnpoint_geo;

DROP TABLE IF EXISTS test_tnpoint_idx;
CREATE TABLE test_tnpoint_idx(op text, no_idx text, rtree_idx text, quadtree_idx text,
  kdtree_idx text, rtree_probe boolean, quadtree_probe boolean,
  kdtree_probe boolean);

INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('<< stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('<< stbox x', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('<< tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp << t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&< stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&< stbox x', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&< tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &< t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&& tstzspan', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp && t2.t)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&& stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&& stbox x', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&& tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&> stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&> stbox x', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&> tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &> t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('>> stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('>> stbox x', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('>> tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp >> t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('~= tstzspan', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('~= stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('~= stbox x', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('~= tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('@> tstzspan', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp @> t2.t)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('@> stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('@> stbox x', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('@> tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp @> t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('<@ tstzspan', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp <@ t2.t)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('<@ stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('<@ stbox x', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('<@ tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&<| stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&<| stbox x', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&<| tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &<| t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('<<| stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('<<| stbox x', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('<<| tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <<| t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('|>> stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('|>> stbox x', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('|>> tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp |>> t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('|&> stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('|&> stbox x', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('|&> tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp |&> t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('-|- tstzspan', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp -|- t2.t)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('-|- stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('-|- stbox x', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('-|- tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&<# tstzspan', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&<# stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &<# t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('&<# tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('<<# tstzspan', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('<<# stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <<# t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('<<# tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('#>> tstzspan', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('#>> stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp #>> t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('#>> tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('#&> tstzspan', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('#&> stbox', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp #&> t2.b)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('#&> tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('stbox >> tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b >> t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('tstzspan && tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t && t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('stbox && tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b && t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('stbox << tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b << t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('tstzspan ~= tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t ~= t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('stbox ~= tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b ~= t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('tstzspan <@ tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t <@ t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('stbox <@ tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <@ t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('tstzspan @> tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t @> t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('stbox @> tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b @> t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('stbox |>> tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b |>> t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('stbox <<| tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <<| t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('tstzspan -|- tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t -|- t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('stbox -|- tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b -|- t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('tstzspan #>> tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t #>> t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('stbox #>> tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b #>> t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('tstzspan <<# tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t <<# t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('stbox <<# tnpoint', (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <<# t1.temp)::text);
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('|=| stbox', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tnpoint_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn));
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('|=| tnpoint', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tnpoint_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn));
INSERT INTO test_tnpoint_idx(op, no_idx) VALUES ('|=| geometry', (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tnpoint_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn));

-- With no index a plan cannot probe one, whatever the planner settings
SET enable_seqscan = off;
SELECT test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp << t2.b') AS no_index_probe;
RESET enable_seqscan;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_rtree ON tbl_tnpoint_idx USING gist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp << t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &< t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp && t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp && t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp >> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp ~= t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp @> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp @> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp <@ t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp |>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp |&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp -|- t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp &<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &<# t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp <<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <<# t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp #>> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp #>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp #&> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp #&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b >> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b >> t1.temp')
WHERE op = 'stbox >> tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t && t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b && t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'stbox && tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b << t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b << t1.temp')
WHERE op = 'stbox << tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t ~= t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b ~= t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'stbox ~= tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t <@ t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <@ t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'stbox <@ tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t @> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t @> t1.temp')
WHERE op = 'tstzspan @> tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b @> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'stbox @> tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b |>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = 'stbox |>> tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <<| t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = 'stbox <<| tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t -|- t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b -|- t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = 'stbox -|- tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t #>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b #>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'stbox #>> tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t <<# t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <<# t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'stbox <<# tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tnpoint_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  rtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tnpoint_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| stbox';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tnpoint_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  rtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tnpoint_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| tnpoint';
UPDATE test_tnpoint_idx SET rtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tnpoint_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn),
  rtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tnpoint_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| geometry';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_rtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_quadtree ON tbl_tnpoint_idx USING spgist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp << t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp && t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp && t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp @> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b >> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b >> t1.temp')
WHERE op = 'stbox >> tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t && t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b && t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'stbox && tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b << t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b << t1.temp')
WHERE op = 'stbox << tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t ~= t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b ~= t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'stbox ~= tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t <@ t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <@ t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'stbox <@ tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t @> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t @> t1.temp')
WHERE op = 'tstzspan @> tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b @> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'stbox @> tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b |>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = 'stbox |>> tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <<| t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = 'stbox <<| tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t -|- t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b -|- t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = 'stbox -|- tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t #>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b #>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'stbox #>> tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t <<# t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <<# t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'stbox <<# tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tnpoint_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  quadtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tnpoint_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| stbox';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tnpoint_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  quadtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tnpoint_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| tnpoint';
UPDATE test_tnpoint_idx SET quadtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tnpoint_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn),
  quadtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tnpoint_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| geometry';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_quadtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_idx_kdtree ON tbl_tnpoint_idx USING spgist(temp tnpoint_kdtree_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp << t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp && t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp && t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp @> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_idx t1, tbl_tnpoint_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b >> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b >> t1.temp')
WHERE op = 'stbox >> tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t && t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b && t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'stbox && tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b << t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b << t1.temp')
WHERE op = 'stbox << tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t ~= t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t ~= t1.temp')
WHERE op = 'tstzspan ~= tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b ~= t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'stbox ~= tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t <@ t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <@ t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'stbox <@ tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t @> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t @> t1.temp')
WHERE op = 'tstzspan @> tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b @> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'stbox @> tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b |>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = 'stbox |>> tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <<| t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = 'stbox <<| tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t -|- t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t -|- t1.temp')
WHERE op = 'tstzspan -|- tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b -|- t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b -|- t1.temp')
WHERE op = 'stbox -|- tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t #>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b #>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'stbox #>> tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t <<# t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_span t2, tbl_tnpoint_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <<# t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_box t2, tbl_tnpoint_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'stbox <<# tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tnpoint_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn),
  kdtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tnpoint_box t2, LATERAL (SELECT t1.temp |=| t2.b AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.b LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| stbox';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tnpoint_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn),
  kdtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tnpoint_val t2, LATERAL (SELECT t1.temp |=| t2.temp AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.temp LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| tnpoint';
UPDATE test_tnpoint_idx SET kdtree_idx = (SELECT string_agg(s.d::text, ',' ORDER BY s.d) FROM tbl_tnpoint_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn),
  kdtree_probe = test_probes_index('SELECT string_agg(s.d::text, '','' ORDER BY s.d) FROM tbl_tnpoint_geo t2, LATERAL (SELECT t1.temp |=| t2.g AS d FROM tbl_tnpoint_idx t1 ORDER BY t1.temp |=| t2.g LIMIT 3) s WHERE t2.knn')
WHERE op = '|=| geometry';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tnpoint_idx_kdtree;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tnpoint_idx;
SELECT COUNT(*) FROM test_tnpoint_idx;
SELECT op, no_idx FROM test_tnpoint_idx WHERE op NOT LIKE '%|=|%' ORDER BY op;
-- The distances of the nearest values are compared, not stated
SELECT op FROM test_tnpoint_idx WHERE op LIKE '%|=|%' AND no_idx IS NULL ORDER BY op;
SELECT * FROM test_tnpoint_idx
WHERE no_idx IS DISTINCT FROM rtree_idx OR no_idx IS DISTINCT FROM quadtree_idx OR
  no_idx IS DISTINCT FROM kdtree_idx OR NOT rtree_probe OR NOT quadtree_probe OR
  NOT kdtree_probe
ORDER BY op;

DROP TABLE test_tnpoint_idx;
DROP TABLE tbl_tnpoint_geo;
DROP TABLE tbl_tnpoint_span;
DROP TABLE tbl_tnpoint_xbox;
DROP TABLE tbl_tnpoint_box;
DROP TABLE tbl_tnpoint_val;
DROP TABLE tbl_tnpoint_src;
DROP TABLE tbl_tnpoint_idx;
-- The distances of the nearest values of the fixture to a value on a route, with
-- the distance read along the route
ANALYZE tbl_tnpoint;

CREATE INDEX tbl_tnpoint_rtree_idx ON tbl_tnpoint USING GIST(temp);

WITH test AS (
  SELECT temp |=| tnpoint '[Npoint(1, 0.2)@2001-06-01, Npoint(1, 0.5)@2001-07-01]' AS distance
  FROM tbl_tnpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| npoint 'Npoint(1, 0.4)' AS distance
  FROM tbl_tnpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| stbox 'SRID=5676;STBOX XT(((0,0),(50,50)),[2001-06-01, 2001-07-01])' AS distance
  FROM tbl_tnpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

DROP INDEX tbl_tnpoint_rtree_idx;

CREATE INDEX tbl_tnpoint_quadtree_idx ON tbl_tnpoint USING SPGIST(temp);

WITH test AS (
  SELECT temp |=| tnpoint '[Npoint(1, 0.2)@2001-06-01, Npoint(1, 0.5)@2001-07-01]' AS distance
  FROM tbl_tnpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| npoint 'Npoint(1, 0.4)' AS distance
  FROM tbl_tnpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| stbox 'SRID=5676;STBOX XT(((0,0),(50,50)),[2001-06-01, 2001-07-01])' AS distance
  FROM tbl_tnpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

DROP INDEX tbl_tnpoint_quadtree_idx;

-- Every row carries the same value, so a split of the quad-tree finds keys that
-- are all equal
CREATE TABLE tbl_tnpoint_allthesame AS
SELECT k, tnpoint(npoint 'Npoint(1, 0.5)', t) AS temp
FROM tbl_tstzspan_big;
CREATE INDEX tbl_tnpoint_allthesame_quadtree ON tbl_tnpoint_allthesame USING spgist(temp);
SELECT COUNT(*) FROM tbl_tnpoint_allthesame WHERE temp && (SELECT stbox(format('SRID=5676;STBOX X((%s,%s),(%s,%s))', xmin(b), ymin(b), xmax(b), ymax(b))) FROM (SELECT temp::stbox AS b FROM tbl_tnpoint_allthesame WHERE temp IS NOT NULL LIMIT 1) s);
SET enable_seqscan = off;
SELECT test_probes_index('SELECT COUNT(*) FROM tbl_tnpoint_allthesame WHERE temp && (SELECT stbox(format(''SRID=5676;STBOX X((%s,%s),(%s,%s))'', xmin(b), ymin(b), xmax(b), ymax(b))) FROM (SELECT temp::stbox AS b FROM tbl_tnpoint_allthesame WHERE temp IS NOT NULL LIMIT 1) s)') AS probe;
SELECT COUNT(*) FROM tbl_tnpoint_allthesame WHERE temp && (SELECT stbox(format('SRID=5676;STBOX X((%s,%s),(%s,%s))', xmin(b), ymin(b), xmax(b), ymax(b))) FROM (SELECT temp::stbox AS b FROM tbl_tnpoint_allthesame WHERE temp IS NOT NULL LIMIT 1) s);
RESET enable_seqscan;
DROP TABLE tbl_tnpoint_allthesame;

DROP FUNCTION test_probes_index(text);

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- A column of stbox values indexed by GiST, quad-tree and k-d tree answers the
-- queries of a tnpoint as the scan does: <<, &<, &&, &>, >>, ~=, @>, <@, &<|, <<|, |>>, |&>, -|-, &<#, <<#, #>>, #&>, |=|
-------------------------------------------------------------------------------

CREATE TABLE tbl_stbox_tnpoint AS
SELECT k, temp::stbox AS b FROM tbl_tnpoint WHERE temp IS NOT NULL;
CREATE TABLE tbl_tnpoint_q AS
SELECT temp AS q FROM tbl_tnpoint WHERE temp IS NOT NULL ORDER BY k LIMIT 3;
CREATE FUNCTION stbox_tnpoint_answers() RETURNS text AS $$
  SELECT string_agg(coalesce(c::text, 'null'), ',' ORDER BY o) FROM (
    SELECT 1, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b << q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 2, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b &< q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 3, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b && q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 4, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b &> q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 5, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b >> q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 6, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b ~= q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 7, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b @> q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 8, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b <@ q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 9, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b &<| q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 10, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b <<| q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 11, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b |>> q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 12, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b |&> q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 13, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b -|- q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 14, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b &<# q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 15, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b <<# q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 16, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b #>> q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 17, (SELECT count(*) FROM tbl_stbox_tnpoint WHERE b #&> q) FROM tbl_tnpoint_q
    UNION ALL
    SELECT 18, (SELECT sum(k) FROM (SELECT k FROM tbl_stbox_tnpoint
      ORDER BY b |=| q, k LIMIT 5) s) FROM tbl_tnpoint_q) t(o, c);
$$ LANGUAGE SQL;
CREATE FUNCTION stbox_tnpoint_probes_index(query text)
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
CREATE FUNCTION stbox_tnpoint_indexed() RETURNS text AS $$
  SELECT string_agg(CASE WHEN stbox_tnpoint_probes_index(query) THEN 't' ELSE 'f' END, '' ORDER BY o)
  FROM (VALUES
    (1, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b << (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (2, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b &< (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (3, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b && (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (4, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b &> (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (5, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b >> (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (6, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b ~= (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (7, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b @> (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (8, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b <@ (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (9, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b &<| (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (10, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b <<| (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (11, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b |>> (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (12, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b |&> (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (13, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b -|- (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (14, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b &<# (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (15, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b <<# (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (16, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b #>> (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (17, 'SELECT count(*) FROM tbl_stbox_tnpoint WHERE b #&> (SELECT q FROM tbl_tnpoint_q LIMIT 1)'),
    (18, 'SELECT k FROM tbl_stbox_tnpoint ORDER BY b |=| (SELECT q FROM tbl_tnpoint_q LIMIT 1) LIMIT 5')) t(o, query);
$$ LANGUAGE SQL;
CREATE TABLE tbl_stbox_tnpoint_seq AS SELECT stbox_tnpoint_answers() AS a;
SELECT a FROM tbl_stbox_tnpoint_seq;
SET enable_seqscan = off;
CREATE INDEX tbl_stbox_tnpoint_gist_idx ON tbl_stbox_tnpoint USING gist(b);
SELECT stbox_tnpoint_indexed(), stbox_tnpoint_answers() = a
FROM tbl_stbox_tnpoint_seq;
DROP INDEX tbl_stbox_tnpoint_gist_idx;
CREATE INDEX tbl_stbox_tnpoint_quad_idx ON tbl_stbox_tnpoint USING spgist(b);
SELECT stbox_tnpoint_indexed(), stbox_tnpoint_answers() = a
FROM tbl_stbox_tnpoint_seq;
DROP INDEX tbl_stbox_tnpoint_quad_idx;
CREATE INDEX tbl_stbox_tnpoint_kd_idx ON tbl_stbox_tnpoint
  USING spgist(b stbox_kdtree_ops);
SELECT stbox_tnpoint_indexed(), stbox_tnpoint_answers() = a
FROM tbl_stbox_tnpoint_seq;
RESET enable_seqscan;
DROP TABLE tbl_stbox_tnpoint, tbl_tnpoint_q, tbl_stbox_tnpoint_seq;
DROP FUNCTION stbox_tnpoint_indexed(), stbox_tnpoint_answers(), stbox_tnpoint_probes_index(text);
