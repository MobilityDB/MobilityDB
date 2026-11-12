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
-- The R-tree GiST, quad-tree and k-d tree SP-GiST indexes of a poseset, keyed
-- by the box the set stores. Under each index every operator answers the
-- rows a sequential scan gives, from a plan that probes the index.
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

DROP TABLE IF EXISTS tbl_poseset_idx;
-- The sets and the union of every two consecutive ones, so that sets of
-- distinct rows overlap and contain one another
CREATE TABLE tbl_poseset_idx AS
WITH src AS (
SELECT k, s FROM tbl_poseset2d)
SELECT k, s FROM src
UNION ALL
SELECT t1.k + 100000, t1.s + t2.s FROM src t1, src t2 WHERE t2.k = t1.k + 1;
ANALYZE tbl_poseset_idx;

DROP TABLE IF EXISTS test_poseset_idx;
CREATE TABLE test_poseset_idx(op text, no_idx text, rtree_idx text, quadtree_idx text,
  kdtree_idx text, rtree_probe boolean, quadtree_probe boolean,
  kdtree_probe boolean);

INSERT INTO test_poseset_idx(op, no_idx) VALUES ('&&', (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s && t2.s)::text);
INSERT INTO test_poseset_idx(op, no_idx) VALUES ('=', (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s = t2.s)::text);
INSERT INTO test_poseset_idx(op, no_idx) VALUES ('@> set', (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s @> t2.s)::text);
INSERT INTO test_poseset_idx(op, no_idx) VALUES ('<@', (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s <@ t2.s)::text);
INSERT INTO test_poseset_idx(op, no_idx) VALUES ('@> value', (SELECT COUNT(*) FROM tbl_poseset_idx t1, (SELECT startValue(s) AS v FROM tbl_poseset_idx) t2 WHERE t1.s @> t2.v)::text);
INSERT INTO test_poseset_idx(op, no_idx) VALUES ('<->', (SELECT COUNT(DISTINCT d)::text || ':' || md5(array_agg(d ORDER BY rn)::text) FROM (SELECT d, row_number() OVER () AS rn FROM (SELECT round((s <-> (SELECT startValue(s) FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY k LIMIT 1))::numeric, 6) AS d FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY s <-> (SELECT startValue(s) FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY k LIMIT 1)) t1) t2)::text);

-------------------------------------------------------------------------------

CREATE INDEX tbl_poseset_idx_rtree ON tbl_poseset_idx USING gist(s);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_poseset_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s && t2.s)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s && t2.s')
WHERE op = '&&';
UPDATE test_poseset_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s = t2.s)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s = t2.s')
WHERE op = '=';
UPDATE test_poseset_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s @> t2.s)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s @> t2.s')
WHERE op = '@> set';
UPDATE test_poseset_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s <@ t2.s)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s <@ t2.s')
WHERE op = '<@';
UPDATE test_poseset_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, (SELECT startValue(s) AS v FROM tbl_poseset_idx) t2 WHERE t1.s @> t2.v)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, (SELECT startValue(s) AS v FROM tbl_poseset_idx) t2 WHERE t1.s @> t2.v')
WHERE op = '@> value';
UPDATE test_poseset_idx SET rtree_idx = (SELECT COUNT(DISTINCT d)::text || ':' || md5(array_agg(d ORDER BY rn)::text) FROM (SELECT d, row_number() OVER () AS rn FROM (SELECT round((s <-> (SELECT startValue(s) FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY k LIMIT 1))::numeric, 6) AS d FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY s <-> (SELECT startValue(s) FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY k LIMIT 1)) t1) t2)::text,
  rtree_probe = test_probes_index('SELECT COUNT(DISTINCT d)::text || '':'' || md5(array_agg(d ORDER BY rn)::text) FROM (SELECT d, row_number() OVER () AS rn FROM (SELECT round((s <-> (SELECT startValue(s) FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY k LIMIT 1))::numeric, 6) AS d FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY s <-> (SELECT startValue(s) FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY k LIMIT 1)) t1) t2')
WHERE op = '<->';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_poseset_idx_rtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_poseset_idx_quadtree ON tbl_poseset_idx USING spgist(s);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_poseset_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s && t2.s)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s && t2.s')
WHERE op = '&&';
UPDATE test_poseset_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s = t2.s)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s = t2.s')
WHERE op = '=';
UPDATE test_poseset_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s @> t2.s)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s @> t2.s')
WHERE op = '@> set';
UPDATE test_poseset_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s <@ t2.s)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s <@ t2.s')
WHERE op = '<@';
UPDATE test_poseset_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, (SELECT startValue(s) AS v FROM tbl_poseset_idx) t2 WHERE t1.s @> t2.v)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, (SELECT startValue(s) AS v FROM tbl_poseset_idx) t2 WHERE t1.s @> t2.v')
WHERE op = '@> value';
UPDATE test_poseset_idx SET quadtree_idx = (SELECT COUNT(DISTINCT d)::text || ':' || md5(array_agg(d ORDER BY rn)::text) FROM (SELECT d, row_number() OVER () AS rn FROM (SELECT round((s <-> (SELECT startValue(s) FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY k LIMIT 1))::numeric, 6) AS d FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY s <-> (SELECT startValue(s) FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY k LIMIT 1)) t1) t2)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(DISTINCT d)::text || '':'' || md5(array_agg(d ORDER BY rn)::text) FROM (SELECT d, row_number() OVER () AS rn FROM (SELECT round((s <-> (SELECT startValue(s) FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY k LIMIT 1))::numeric, 6) AS d FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY s <-> (SELECT startValue(s) FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY k LIMIT 1)) t1) t2')
WHERE op = '<->';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_poseset_idx_quadtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_poseset_idx_kdtree ON tbl_poseset_idx USING spgist(s poseset_kdtree_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_poseset_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s && t2.s)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s && t2.s')
WHERE op = '&&';
UPDATE test_poseset_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s = t2.s)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s = t2.s')
WHERE op = '=';
UPDATE test_poseset_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s @> t2.s)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s @> t2.s')
WHERE op = '@> set';
UPDATE test_poseset_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s <@ t2.s)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, tbl_poseset_idx t2 WHERE t1.s <@ t2.s')
WHERE op = '<@';
UPDATE test_poseset_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_poseset_idx t1, (SELECT startValue(s) AS v FROM tbl_poseset_idx) t2 WHERE t1.s @> t2.v)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_poseset_idx t1, (SELECT startValue(s) AS v FROM tbl_poseset_idx) t2 WHERE t1.s @> t2.v')
WHERE op = '@> value';
UPDATE test_poseset_idx SET kdtree_idx = (SELECT COUNT(DISTINCT d)::text || ':' || md5(array_agg(d ORDER BY rn)::text) FROM (SELECT d, row_number() OVER () AS rn FROM (SELECT round((s <-> (SELECT startValue(s) FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY k LIMIT 1))::numeric, 6) AS d FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY s <-> (SELECT startValue(s) FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY k LIMIT 1)) t1) t2)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(DISTINCT d)::text || '':'' || md5(array_agg(d ORDER BY rn)::text) FROM (SELECT d, row_number() OVER () AS rn FROM (SELECT round((s <-> (SELECT startValue(s) FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY k LIMIT 1))::numeric, 6) AS d FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY s <-> (SELECT startValue(s) FROM tbl_poseset_idx WHERE s IS NOT NULL ORDER BY k LIMIT 1)) t1) t2')
WHERE op = '<->';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_poseset_idx_kdtree;

-------------------------------------------------------------------------------

SELECT op, split_part(no_idx, ':', 1) AS no_idx FROM test_poseset_idx ORDER BY op;
SELECT * FROM test_poseset_idx
WHERE no_idx IS DISTINCT FROM rtree_idx OR no_idx IS DISTINCT FROM quadtree_idx OR
  no_idx IS DISTINCT FROM kdtree_idx OR NOT rtree_probe OR NOT quadtree_probe OR
  NOT kdtree_probe
ORDER BY op;

DROP TABLE test_poseset_idx;
DROP TABLE tbl_poseset_idx;
DROP FUNCTION test_probes_index(text);

-------------------------------------------------------------------------------
