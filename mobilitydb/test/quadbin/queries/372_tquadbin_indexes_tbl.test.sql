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
-- The R-tree GiST, quad-tree and k-d tree SP-GiST indexes of a tquadbin, keyed
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

DROP TABLE IF EXISTS tbl_tquadbin_idx;
CREATE TABLE tbl_tquadbin_idx AS SELECT k, temp FROM tbl_tquadbin_big;
ANALYZE tbl_tquadbin_idx;

-- The values, boxes and periods the operators are given. Those of the fixture
-- are random, so they meet few values exactly: the others come from the rows of
-- the table, so that every operator has rows to answer.
DROP TABLE IF EXISTS tbl_tquadbin_src;
CREATE TABLE tbl_tquadbin_src AS
SELECT k, temp, temp::stbox AS b, temp::tstzspan AS t
FROM tbl_tquadbin_idx WHERE k % 1000 = 0;

DROP TABLE IF EXISTS tbl_tquadbin_val;
CREATE TABLE tbl_tquadbin_val AS
SELECT temp FROM tbl_tquadbin WHERE k % 20 = 0
UNION ALL
SELECT temp FROM tbl_tquadbin_src
UNION ALL
SELECT shiftTime(temp, lower(t) - upper(t)) FROM tbl_tquadbin_src;
ANALYZE tbl_tquadbin_val;

-- A box of the fixture, the box of a value, one around it, one inside it, and
-- one whose period meets it
DROP TABLE IF EXISTS tbl_tquadbin_box;
CREATE TABLE tbl_tquadbin_box AS
SELECT b FROM tbl_stbox_quadbin WHERE k % 20 = 0
UNION ALL
SELECT b FROM tbl_tquadbin_src
UNION ALL
SELECT expandTime(expandSpace(b, 0.1), interval '1 day') FROM tbl_tquadbin_src
UNION ALL
SELECT stbox(format('SRID=4326;STBOX XT(((%s,%s),(%s,%s)),[%s,%s])',
    (xmin(b) + xmax(b)) / 2, (ymin(b) + ymax(b)) / 2, (xmin(b) + xmax(b)) / 2 + 0.000001, (ymin(b) + ymax(b)) / 2 + 0.000001, lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2))
FROM tbl_tquadbin_src
UNION ALL
SELECT stbox(format('SRID=4326;STBOX XT(((%s,%s),(%s,%s)),[%s,%s%s)',
    xmin(b), ymin(b), xmax(b), ymax(b), lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END))
FROM tbl_tquadbin_src;
ANALYZE tbl_tquadbin_box;

-- A box of space alone, which the operators on time refuse: one around a value,
-- one inside it, the space of a value, and one that meets it
DROP TABLE IF EXISTS tbl_tquadbin_xbox;
CREATE TABLE tbl_tquadbin_xbox AS
SELECT stbox(format('SRID=4326;STBOX X((%s,%s),(%s,%s))',
    xmin(b) - 0.1, ymin(b) - 0.1, xmax(b) + 0.1, ymax(b) + 0.1)) AS b
FROM tbl_tquadbin_src
UNION ALL
SELECT stbox(format('SRID=4326;STBOX X((%s,%s),(%s,%s))',
    (xmin(b) + xmax(b)) / 2, (ymin(b) + ymax(b)) / 2, (xmin(b) + xmax(b)) / 2 + 0.000001, (ymin(b) + ymax(b)) / 2 + 0.000001))
FROM tbl_tquadbin_src
UNION ALL
SELECT stbox(format('SRID=4326;STBOX X((%s,%s),(%s,%s))',
    xmin(b), ymin(b), xmax(b), ymax(b)))
FROM tbl_tquadbin_src
UNION ALL
SELECT stbox(format('SRID=4326;STBOX X((%s,%s),(%s,%s))',
    xmax(b), ymin(b), xmax(b) + 0.1, ymax(b)))
FROM tbl_tquadbin_src;
ANALYZE tbl_tquadbin_xbox;

-- A period of the fixture, the period of a value, one around it, one inside it,
-- and one that meets it
DROP TABLE IF EXISTS tbl_tquadbin_span;
CREATE TABLE tbl_tquadbin_span AS
SELECT t FROM tbl_tstzspan WHERE k % 20 = 0
UNION ALL
SELECT t FROM tbl_tquadbin_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) - interval '1 day', upper(t) + interval '1 day'))
FROM tbl_tquadbin_src
UNION ALL
SELECT tstzspan(format('[%s, %s]', lower(t) + (upper(t) - lower(t)) / 2, lower(t) + (upper(t) - lower(t)) / 2))
FROM tbl_tquadbin_src
UNION ALL
SELECT tstzspan(format('[%s, %s%s', lower(t) - interval '1 hour', lower(t),
    CASE WHEN lowerInc(t) THEN ')' ELSE ']' END))
FROM tbl_tquadbin_src;
ANALYZE tbl_tquadbin_span;

DROP TABLE IF EXISTS test_tquadbin_idx;
CREATE TABLE test_tquadbin_idx(op text, no_idx text, rtree_idx text, quadtree_idx text,
  kdtree_idx text, rtree_probe boolean, quadtree_probe boolean,
  kdtree_probe boolean);

INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('<< tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp << t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&< tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &< t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&& tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&> tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &> t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('>> tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp >> t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('~= tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('@> tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp @> t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('<@ tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&<| tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &<| t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('<<| tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <<| t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('|>> tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp |>> t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('|&> tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp |&> t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('-|- tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&<# tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('<<# tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('#>> tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('#&> tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('<< stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&< stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&& stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&> stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('>> stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('~= stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('@> stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('<@ stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&<| stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('<<| stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('|>> stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('|&> stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('-|- stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&<# stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &<# t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('<<# stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <<# t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('#>> stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp #>> t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('#&> stbox', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp #&> t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&& tstzspan', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp && t2.t)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('~= tstzspan', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('@> tstzspan', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp @> t2.t)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('<@ tstzspan', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp <@ t2.t)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('-|- tstzspan', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp -|- t2.t)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&<# tstzspan', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('<<# tstzspan', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('#>> tstzspan', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('#&> tstzspan', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('<< stbox x', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&< stbox x', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&& stbox x', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&> stbox x', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('>> stbox x', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('~= stbox x', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('@> stbox x', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('<@ stbox x', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('&<| stbox x', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('<<| stbox x', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('|>> stbox x', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('|&> stbox x', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('-|- stbox x', (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('stbox && tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b && t1.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('stbox @> tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b @> t1.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('stbox <@ tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <@ t1.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('stbox ~= tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b ~= t1.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('tstzspan && tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t && t1.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('tstzspan <@ tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t <@ t1.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('stbox << tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b << t1.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('stbox >> tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b >> t1.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('stbox <<| tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <<| t1.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('stbox |>> tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b |>> t1.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('tstzspan <<# tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t <<# t1.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('tstzspan #>> tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t #>> t1.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('stbox <<# tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <<# t1.temp)::text);
INSERT INTO test_tquadbin_idx(op, no_idx) VALUES ('stbox #>> tquadbin', (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b #>> t1.temp)::text);

-- With no index a plan cannot probe one, whatever the planner settings
SET enable_seqscan = off;
SELECT test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp && t2.b') AS no_index_probe;
RESET enable_seqscan;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tquadbin_idx_rtree ON tbl_tquadbin_idx USING gist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp << t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &< t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp && t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp >> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp @> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp |>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp |&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &<# t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <<# t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp #>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp #&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp && t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp ~= t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp @> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp <@ t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp -|- t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp &<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp <<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp #>> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp #&> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b && t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'stbox && tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b @> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'stbox @> tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <@ t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'stbox <@ tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b ~= t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'stbox ~= tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t && t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t <@ t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b << t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b << t1.temp')
WHERE op = 'stbox << tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b >> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b >> t1.temp')
WHERE op = 'stbox >> tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <<| t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = 'stbox <<| tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b |>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = 'stbox |>> tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t <<# t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t #>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <<# t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'stbox <<# tquadbin';
UPDATE test_tquadbin_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b #>> t1.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'stbox #>> tquadbin';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tquadbin_idx_rtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tquadbin_idx_quadtree ON tbl_tquadbin_idx USING spgist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp << t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp && t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp && t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp @> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b && t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'stbox && tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b @> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'stbox @> tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <@ t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'stbox <@ tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b ~= t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'stbox ~= tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t && t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t <@ t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b << t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b << t1.temp')
WHERE op = 'stbox << tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b >> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b >> t1.temp')
WHERE op = 'stbox >> tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <<| t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = 'stbox <<| tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b |>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = 'stbox |>> tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t <<# t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t #>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <<# t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'stbox <<# tquadbin';
UPDATE test_tquadbin_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b #>> t1.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'stbox #>> tquadbin';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tquadbin_idx_quadtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tquadbin_idx_kdtree ON tbl_tquadbin_idx USING spgist(temp tquadbin_kdtree_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp << t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp && t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_val t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp && t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp @> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_idx t1, tbl_tquadbin_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b && t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b && t1.temp')
WHERE op = 'stbox && tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b @> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b @> t1.temp')
WHERE op = 'stbox @> tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <@ t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <@ t1.temp')
WHERE op = 'stbox <@ tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b ~= t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b ~= t1.temp')
WHERE op = 'stbox ~= tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t && t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t && t1.temp')
WHERE op = 'tstzspan && tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t <@ t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t <@ t1.temp')
WHERE op = 'tstzspan <@ tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b << t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b << t1.temp')
WHERE op = 'stbox << tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b >> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b >> t1.temp')
WHERE op = 'stbox >> tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <<| t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <<| t1.temp')
WHERE op = 'stbox <<| tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b |>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b |>> t1.temp')
WHERE op = 'stbox |>> tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t <<# t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t <<# t1.temp')
WHERE op = 'tstzspan <<# tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t #>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_span t2, tbl_tquadbin_idx t1 WHERE t2.t #>> t1.temp')
WHERE op = 'tstzspan #>> tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <<# t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b <<# t1.temp')
WHERE op = 'stbox <<# tquadbin';
UPDATE test_tquadbin_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b #>> t1.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tquadbin_box t2, tbl_tquadbin_idx t1 WHERE t2.b #>> t1.temp')
WHERE op = 'stbox #>> tquadbin';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tquadbin_idx_kdtree;

-------------------------------------------------------------------------------

-- Every operator has rows to answer. The counts themselves are not stated: two
-- cells can share a box edge that each mathematical library rounds its own way.
SELECT COUNT(*) FROM test_tquadbin_idx;
SELECT op FROM test_tquadbin_idx WHERE no_idx::bigint = 0 ORDER BY op;

SELECT * FROM test_tquadbin_idx
WHERE no_idx IS DISTINCT FROM rtree_idx OR no_idx IS DISTINCT FROM quadtree_idx OR
  no_idx IS DISTINCT FROM kdtree_idx OR NOT rtree_probe OR NOT quadtree_probe OR
  NOT kdtree_probe
ORDER BY op;

DROP TABLE test_tquadbin_idx;
DROP TABLE tbl_tquadbin_span;
DROP TABLE tbl_tquadbin_xbox;
DROP TABLE tbl_tquadbin_box;
DROP TABLE tbl_tquadbin_val;
DROP TABLE tbl_tquadbin_src;
DROP TABLE tbl_tquadbin_idx;
DROP FUNCTION test_probes_index(text);

-------------------------------------------------------------------------------
