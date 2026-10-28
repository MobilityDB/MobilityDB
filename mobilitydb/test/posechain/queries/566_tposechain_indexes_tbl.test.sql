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
-- The R-tree GiST, quad-tree and k-d tree SP-GiST indexes of a tposechain. Under
-- each index every operator answers the rows a sequential scan gives, from a
-- plan that probes the index.
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

DROP TABLE IF EXISTS tbl_tposechain_idx;
CREATE TABLE tbl_tposechain_idx AS SELECT k, temp FROM tbl_tposechain;
ANALYZE tbl_tposechain_idx;

DROP TABLE IF EXISTS test_tposechain_idx;
CREATE TABLE test_tposechain_idx(op text, no_idx text, rtree_idx text, quadtree_idx text,
  kdtree_idx text, rtree_probe boolean, quadtree_probe boolean,
  kdtree_probe boolean);

INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('&&', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('@>', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp @> t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('<@', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('~=', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('-|-', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('<<', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp << t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('&<', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &< t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('>>', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp >> t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('&>', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &> t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('<<|', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <<| t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('&<|', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &<| t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('|>>', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp |>> t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('|&>', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp |&> t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('<<#', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('&<#', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('#>>', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('#&>', (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_tposechain_idx(op, no_idx) VALUES ('|=|', (SELECT COUNT(DISTINCT d)::text || ':' || md5(array_agg(d ORDER BY rn)::text) FROM (SELECT d, row_number() OVER () AS rn FROM (SELECT round((temp |=| geometry 'SRID=3812;Point(0 0)')::numeric, 6) AS d FROM tbl_tposechain_idx WHERE temp IS NOT NULL ORDER BY temp |=| geometry 'SRID=3812;Point(0 0)') t1) t2)::text);

-------------------------------------------------------------------------------

CREATE INDEX tbl_tposechain_idx_rtree ON tbl_tposechain_idx USING gist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp && t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp && t2.temp')
WHERE op = '&&';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp @> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp @> t2.temp')
WHERE op = '@>';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~=';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|-';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp << t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp << t2.temp')
WHERE op = '<<';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &< t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &< t2.temp')
WHERE op = '&<';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp >> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp >> t2.temp')
WHERE op = '>>';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &> t2.temp')
WHERE op = '&>';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<|';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<|';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>>';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&>';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<#';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<#';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>>';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&>';
UPDATE test_tposechain_idx SET rtree_idx = (SELECT COUNT(DISTINCT d)::text || ':' || md5(array_agg(d ORDER BY rn)::text) FROM (SELECT d, row_number() OVER () AS rn FROM (SELECT round((temp |=| geometry 'SRID=3812;Point(0 0)')::numeric, 6) AS d FROM tbl_tposechain_idx WHERE temp IS NOT NULL ORDER BY temp |=| geometry 'SRID=3812;Point(0 0)') t1) t2)::text,
  rtree_probe = test_probes_index('SELECT COUNT(DISTINCT d)::text || '':'' || md5(array_agg(d ORDER BY rn)::text) FROM (SELECT d, row_number() OVER () AS rn FROM (SELECT round((temp |=| geometry ''SRID=3812;Point(0 0)'')::numeric, 6) AS d FROM tbl_tposechain_idx WHERE temp IS NOT NULL ORDER BY temp |=| geometry ''SRID=3812;Point(0 0)'') t1) t2')
WHERE op = '|=|';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tposechain_idx_rtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tposechain_idx_quadtree ON tbl_tposechain_idx USING spgist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp && t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp && t2.temp')
WHERE op = '&&';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp @> t2.temp')
WHERE op = '@>';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~=';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|-';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp << t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp << t2.temp')
WHERE op = '<<';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &< t2.temp')
WHERE op = '&<';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp >> t2.temp')
WHERE op = '>>';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &> t2.temp')
WHERE op = '&>';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<|';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<|';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>>';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&>';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<#';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<#';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>>';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&>';
UPDATE test_tposechain_idx SET quadtree_idx = (SELECT COUNT(DISTINCT d)::text || ':' || md5(array_agg(d ORDER BY rn)::text) FROM (SELECT d, row_number() OVER () AS rn FROM (SELECT round((temp |=| geometry 'SRID=3812;Point(0 0)')::numeric, 6) AS d FROM tbl_tposechain_idx WHERE temp IS NOT NULL ORDER BY temp |=| geometry 'SRID=3812;Point(0 0)') t1) t2)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(DISTINCT d)::text || '':'' || md5(array_agg(d ORDER BY rn)::text) FROM (SELECT d, row_number() OVER () AS rn FROM (SELECT round((temp |=| geometry ''SRID=3812;Point(0 0)'')::numeric, 6) AS d FROM tbl_tposechain_idx WHERE temp IS NOT NULL ORDER BY temp |=| geometry ''SRID=3812;Point(0 0)'') t1) t2')
WHERE op = '|=|';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tposechain_idx_quadtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tposechain_idx_kdtree ON tbl_tposechain_idx USING spgist(temp tposechain_kdtree_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp && t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp && t2.temp')
WHERE op = '&&';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp @> t2.temp')
WHERE op = '@>';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~=';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|-';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp << t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp << t2.temp')
WHERE op = '<<';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &< t2.temp')
WHERE op = '&<';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp >> t2.temp')
WHERE op = '>>';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &> t2.temp')
WHERE op = '&>';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<|';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<|';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>>';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&>';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<#';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<#';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>>';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_tposechain_idx t1, tbl_tposechain_idx t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&>';
UPDATE test_tposechain_idx SET kdtree_idx = (SELECT COUNT(DISTINCT d)::text || ':' || md5(array_agg(d ORDER BY rn)::text) FROM (SELECT d, row_number() OVER () AS rn FROM (SELECT round((temp |=| geometry 'SRID=3812;Point(0 0)')::numeric, 6) AS d FROM tbl_tposechain_idx WHERE temp IS NOT NULL ORDER BY temp |=| geometry 'SRID=3812;Point(0 0)') t1) t2)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(DISTINCT d)::text || '':'' || md5(array_agg(d ORDER BY rn)::text) FROM (SELECT d, row_number() OVER () AS rn FROM (SELECT round((temp |=| geometry ''SRID=3812;Point(0 0)'')::numeric, 6) AS d FROM tbl_tposechain_idx WHERE temp IS NOT NULL ORDER BY temp |=| geometry ''SRID=3812;Point(0 0)'') t1) t2')
WHERE op = '|=|';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tposechain_idx_kdtree;

-------------------------------------------------------------------------------

SELECT op, split_part(no_idx, ':', 1) AS no_idx FROM test_tposechain_idx ORDER BY op;
SELECT * FROM test_tposechain_idx
WHERE no_idx IS DISTINCT FROM rtree_idx OR no_idx IS DISTINCT FROM quadtree_idx OR
  no_idx IS DISTINCT FROM kdtree_idx OR NOT rtree_probe OR NOT quadtree_probe OR
  NOT kdtree_probe
ORDER BY op;

DROP TABLE test_tposechain_idx;
DROP TABLE tbl_tposechain_idx;
DROP FUNCTION test_probes_index(text);

-------------------------------------------------------------------------------
