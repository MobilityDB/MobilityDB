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

-------------------------------------------------------------------------------
-- A column of stbox values indexed by GiST, quad-tree and k-d tree answers the
-- queries of a tposechain as the scan does: <<, &<, &&, &>, >>, ~=, @>, <@, &<|, <<|, |>>, |&>, -|-, &<#, <<#, #>>, #&>, |=|
-------------------------------------------------------------------------------

CREATE TABLE tbl_stbox_tposechain AS
SELECT k, temp::stbox AS b FROM tbl_tposechain WHERE temp IS NOT NULL;
CREATE TABLE tbl_tposechain_q AS
SELECT temp AS q FROM tbl_tposechain WHERE temp IS NOT NULL ORDER BY k LIMIT 3;
CREATE FUNCTION stbox_tposechain_answers() RETURNS text AS $$
  SELECT string_agg(coalesce(c::text, 'null'), ',' ORDER BY o) FROM (
    SELECT 1, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b << q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 2, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b &< q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 3, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b && q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 4, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b &> q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 5, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b >> q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 6, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b ~= q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 7, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b @> q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 8, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b <@ q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 9, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b &<| q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 10, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b <<| q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 11, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b |>> q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 12, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b |&> q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 13, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b -|- q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 14, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b &<# q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 15, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b <<# q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 16, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b #>> q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 17, (SELECT count(*) FROM tbl_stbox_tposechain WHERE b #&> q) FROM tbl_tposechain_q
    UNION ALL
    SELECT 18, (SELECT sum(k) FROM (SELECT k FROM tbl_stbox_tposechain
      ORDER BY b |=| q, k LIMIT 5) s) FROM tbl_tposechain_q) t(o, c);
$$ LANGUAGE SQL;
CREATE FUNCTION stbox_tposechain_probes_index(query text)
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
CREATE FUNCTION stbox_tposechain_indexed() RETURNS text AS $$
  SELECT string_agg(CASE WHEN stbox_tposechain_probes_index(query) THEN 't' ELSE 'f' END, '' ORDER BY o)
  FROM (VALUES
    (1, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b << (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (2, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b &< (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (3, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b && (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (4, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b &> (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (5, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b >> (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (6, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b ~= (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (7, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b @> (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (8, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b <@ (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (9, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b &<| (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (10, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b <<| (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (11, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b |>> (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (12, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b |&> (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (13, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b -|- (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (14, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b &<# (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (15, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b <<# (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (16, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b #>> (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (17, 'SELECT count(*) FROM tbl_stbox_tposechain WHERE b #&> (SELECT q FROM tbl_tposechain_q LIMIT 1)'),
    (18, 'SELECT k FROM tbl_stbox_tposechain ORDER BY b |=| (SELECT q FROM tbl_tposechain_q LIMIT 1) LIMIT 5')) t(o, query);
$$ LANGUAGE SQL;
CREATE TABLE tbl_stbox_tposechain_seq AS SELECT stbox_tposechain_answers() AS a;
SELECT a FROM tbl_stbox_tposechain_seq;
SET enable_seqscan = off;
CREATE INDEX tbl_stbox_tposechain_gist_idx ON tbl_stbox_tposechain USING gist(b);
SELECT stbox_tposechain_indexed(), stbox_tposechain_answers() = a
FROM tbl_stbox_tposechain_seq;
DROP INDEX tbl_stbox_tposechain_gist_idx;
CREATE INDEX tbl_stbox_tposechain_quad_idx ON tbl_stbox_tposechain USING spgist(b);
SELECT stbox_tposechain_indexed(), stbox_tposechain_answers() = a
FROM tbl_stbox_tposechain_seq;
DROP INDEX tbl_stbox_tposechain_quad_idx;
CREATE INDEX tbl_stbox_tposechain_kd_idx ON tbl_stbox_tposechain
  USING spgist(b stbox_kdtree_ops);
SELECT stbox_tposechain_indexed(), stbox_tposechain_answers() = a
FROM tbl_stbox_tposechain_seq;
RESET enable_seqscan;
DROP TABLE tbl_stbox_tposechain, tbl_tposechain_q, tbl_stbox_tposechain_seq;
DROP FUNCTION stbox_tposechain_indexed(), stbox_tposechain_answers(), stbox_tposechain_probes_index(text);
