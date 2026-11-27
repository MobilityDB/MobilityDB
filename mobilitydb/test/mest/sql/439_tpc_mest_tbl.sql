-------------------------------------------------------------------------------
-- The multi-entry R-tree mgist, quad-tree and k-d tree mspgist indexes of a
-- tpcpoint, keyed by the boxes of its pieces and of the whole value, one per
-- split. Under each index every operator the class holds, with each right argument
-- of a fixture, answers the rows a sequential scan gives, from a plan that probes
-- the index.
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

DROP TABLE IF EXISTS tbl_tpcpoint_idx;
CREATE TEMP TABLE tbl_tpcpoint_idx AS TABLE tbl_tpcpoint;
ANALYZE tbl_tpcpoint_idx;

DROP TABLE IF EXISTS test_tpcpoint_mest;
CREATE TEMP TABLE test_tpcpoint_mest(op text, no_idx text,
  rtree_equi_idx text, rtree_equi_probe boolean,
  rtree_seg_idx text, rtree_seg_probe boolean,
  rtree_merge_idx text, rtree_merge_probe boolean,
  rtree_adapt_idx text, rtree_adapt_probe boolean,
  quadtree_equi_idx text, quadtree_equi_probe boolean,
  quadtree_seg_idx text, quadtree_seg_probe boolean,
  quadtree_merge_idx text, quadtree_merge_probe boolean,
  quadtree_adapt_idx text, quadtree_adapt_probe boolean,
  kdtree_equi_idx text, kdtree_equi_probe boolean,
  kdtree_seg_idx text, kdtree_seg_probe boolean,
  kdtree_merge_idx text, kdtree_merge_probe boolean,
  kdtree_adapt_idx text, kdtree_adapt_probe boolean);

INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('<< tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('<< tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('&< tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('&< tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('&& tstzspan', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('&& tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('&& tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('&> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('&> tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('>> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('>> tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('~= tstzspan', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('~= tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('~= tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('@> tstzspan', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('@> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('@> tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('<@ tstzspan', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('<@ tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('<@ tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('&<| tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('&<| tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('<<| tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('<<| tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('|>> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('|>> tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('|&> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('|&> tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('-|- tstzspan', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('-|- tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('-|- tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('&<# tstzspan', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('&<# tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('&<# tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('<<# tstzspan', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('<<# tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('<<# tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('#>> tstzspan', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('#>> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('#>> tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('#&> tstzspan', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('#&> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('#&> tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('&</ tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('&</ tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('<</ tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('<</ tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('/>> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('/>> tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('/&> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('/&> tpcpoint', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('?&& tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text);
INSERT INTO test_tpcpoint_mest(op, no_idx) VALUES ('%&& tpcbox', (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text);

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_idx_rtree_equi ON tbl_tpcpoint_idx
  USING mgist(temp tpcpoint_mrtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpoint_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpoint_idx_rtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_idx_rtree_seg ON tbl_tpcpoint_idx
  USING mgist(temp tpcpoint_mrtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpoint_idx_rtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_idx_rtree_merge ON tbl_tpcpoint_idx
  USING mgist(temp tpcpoint_mrtree_mergesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpoint_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpoint_idx_rtree_merge;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_idx_rtree_adapt ON tbl_tpcpoint_idx
  USING mgist(temp tpcpoint_mrtree_adaptsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpoint';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpoint_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpoint_idx_rtree_adapt;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_idx_quadtree_equi ON tbl_tpcpoint_idx
  USING mspgist(temp tpcpoint_mquadtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpoint_idx_quadtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_idx_quadtree_seg ON tbl_tpcpoint_idx
  USING mspgist(temp tpcpoint_mquadtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpoint_idx_quadtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_idx_quadtree_merge ON tbl_tpcpoint_idx
  USING mspgist(temp tpcpoint_mquadtree_mergesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpoint_idx_quadtree_merge;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_idx_quadtree_adapt ON tbl_tpcpoint_idx
  USING mspgist(temp tpcpoint_mquadtree_adaptsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpoint';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpoint_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpoint_idx_quadtree_adapt;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_idx_kdtree_equi ON tbl_tpcpoint_idx
  USING mspgist(temp tpcpoint_mkdtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpoint_idx_kdtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_idx_kdtree_seg ON tbl_tpcpoint_idx
  USING mspgist(temp tpcpoint_mkdtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpoint_idx_kdtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_idx_kdtree_merge ON tbl_tpcpoint_idx
  USING mspgist(temp tpcpoint_mkdtree_mergesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpoint_idx_kdtree_merge;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_idx_kdtree_adapt ON tbl_tpcpoint_idx
  USING mspgist(temp tpcpoint_mkdtree_adaptsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcpoint t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpoint';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpoint_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpoint_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpoint_idx_kdtree_adapt;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpcpoint_idx;
SELECT COUNT(*) FROM test_tpcpoint_mest;
SELECT op, no_idx FROM test_tpcpoint_mest ORDER BY op;
SELECT * FROM test_tpcpoint_mest
WHERE no_idx IS DISTINCT FROM rtree_equi_idx OR NOT rtree_equi_probe OR
  no_idx IS DISTINCT FROM rtree_seg_idx OR NOT rtree_seg_probe OR
  no_idx IS DISTINCT FROM rtree_merge_idx OR NOT rtree_merge_probe OR
  no_idx IS DISTINCT FROM rtree_adapt_idx OR NOT rtree_adapt_probe OR
  no_idx IS DISTINCT FROM quadtree_equi_idx OR NOT quadtree_equi_probe OR
  no_idx IS DISTINCT FROM quadtree_seg_idx OR NOT quadtree_seg_probe OR
  no_idx IS DISTINCT FROM quadtree_merge_idx OR NOT quadtree_merge_probe OR
  no_idx IS DISTINCT FROM quadtree_adapt_idx OR NOT quadtree_adapt_probe OR
  no_idx IS DISTINCT FROM kdtree_equi_idx OR NOT kdtree_equi_probe OR
  no_idx IS DISTINCT FROM kdtree_seg_idx OR NOT kdtree_seg_probe OR
  no_idx IS DISTINCT FROM kdtree_merge_idx OR NOT kdtree_merge_probe OR
  no_idx IS DISTINCT FROM kdtree_adapt_idx OR NOT kdtree_adapt_probe
ORDER BY op;

DROP TABLE test_tpcpoint_mest;
DROP TABLE tbl_tpcpoint_idx;

DROP FUNCTION pg_temp.test_probes_index(text);

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- The multi-entry R-tree mgist, quad-tree and k-d tree mspgist indexes of a
-- tpcpatch, keyed by the boxes of its pieces and of the whole value, one per
-- split. Under each index every operator the class holds, with each right argument
-- of a fixture, answers the rows a sequential scan gives, from a plan that probes
-- the index.
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

DROP TABLE IF EXISTS tbl_tpcpatch_idx;
CREATE TEMP TABLE tbl_tpcpatch_idx AS TABLE tbl_tpcpatch;
ANALYZE tbl_tpcpatch_idx;

DROP TABLE IF EXISTS test_tpcpatch_mest;
CREATE TEMP TABLE test_tpcpatch_mest(op text, no_idx text,
  rtree_equi_idx text, rtree_equi_probe boolean,
  rtree_seg_idx text, rtree_seg_probe boolean,
  rtree_merge_idx text, rtree_merge_probe boolean,
  rtree_adapt_idx text, rtree_adapt_probe boolean,
  quadtree_equi_idx text, quadtree_equi_probe boolean,
  quadtree_seg_idx text, quadtree_seg_probe boolean,
  quadtree_merge_idx text, quadtree_merge_probe boolean,
  quadtree_adapt_idx text, quadtree_adapt_probe boolean,
  kdtree_equi_idx text, kdtree_equi_probe boolean,
  kdtree_seg_idx text, kdtree_seg_probe boolean,
  kdtree_merge_idx text, kdtree_merge_probe boolean,
  kdtree_adapt_idx text, kdtree_adapt_probe boolean);

INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('<< tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('<< tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('&< tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('&< tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('&& tstzspan', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('&& tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('&& tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('&> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('&> tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('>> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('>> tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('~= tstzspan', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('~= tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('~= tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('@> tstzspan', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('@> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('@> tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('<@ tstzspan', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('<@ tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('<@ tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('&<| tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('&<| tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('<<| tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('<<| tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('|>> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('|>> tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('|&> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('|&> tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('-|- tstzspan', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('-|- tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('-|- tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('&<# tstzspan', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('&<# tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('&<# tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('<<# tstzspan', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('<<# tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('<<# tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('#>> tstzspan', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('#>> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('#>> tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('#&> tstzspan', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('#&> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('#&> tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('&</ tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('&</ tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('<</ tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('<</ tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('/>> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('/>> tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('/&> tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('/&> tpcpatch', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('?&& tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text);
INSERT INTO test_tpcpatch_mest(op, no_idx) VALUES ('%&& tpcbox', (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text);

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_idx_rtree_equi ON tbl_tpcpatch_idx
  USING mgist(temp tpcpatch_mrtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpatch_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpatch_idx_rtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_idx_rtree_seg ON tbl_tpcpatch_idx
  USING mgist(temp tpcpatch_mrtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpatch_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpatch_idx_rtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_idx_rtree_merge ON tbl_tpcpatch_idx
  USING mgist(temp tpcpatch_mrtree_mergesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpatch_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpatch_idx_rtree_merge;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_idx_rtree_adapt ON tbl_tpcpatch_idx
  USING mgist(temp tpcpatch_mrtree_adaptsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpatch';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpatch_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpatch_idx_rtree_adapt;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_idx_quadtree_equi ON tbl_tpcpatch_idx
  USING mspgist(temp tpcpatch_mquadtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpatch_idx_quadtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_idx_quadtree_seg ON tbl_tpcpatch_idx
  USING mspgist(temp tpcpatch_mquadtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpatch_idx_quadtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_idx_quadtree_merge ON tbl_tpcpatch_idx
  USING mspgist(temp tpcpatch_mquadtree_mergesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpatch_idx_quadtree_merge;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_idx_quadtree_adapt ON tbl_tpcpatch_idx
  USING mspgist(temp tpcpatch_mquadtree_adaptsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpatch';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpatch_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpatch_idx_quadtree_adapt;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_idx_kdtree_equi ON tbl_tpcpatch_idx
  USING mspgist(temp tpcpatch_mkdtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpatch_idx_kdtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_idx_kdtree_seg ON tbl_tpcpatch_idx
  USING mspgist(temp tpcpatch_mkdtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpatch_idx_kdtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_idx_kdtree_merge ON tbl_tpcpatch_idx
  USING mspgist(temp tpcpatch_mkdtree_mergesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpatch_idx_kdtree_merge;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_idx_kdtree_adapt ON tbl_tpcpatch_idx
  USING mspgist(temp tpcpatch_mkdtree_adaptsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp &</ t2.b')
WHERE op = '&</ tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp &</ t2.temp')
WHERE op = '&</ tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp <</ t2.b')
WHERE op = '<</ tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp <</ t2.temp')
WHERE op = '<</ tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp />> t2.b')
WHERE op = '/>> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp />> t2.temp')
WHERE op = '/>> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp /&> t2.b')
WHERE op = '/&> tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcpatch t2 WHERE t1.temp /&> t2.temp')
WHERE op = '/&> tpcpatch';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& tpcbox';
UPDATE test_tpcpatch_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tpcpatch_idx t1, tbl_tpcbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& tpcbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tpcpatch_idx_kdtree_adapt;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpcpatch_idx;
SELECT COUNT(*) FROM test_tpcpatch_mest;
SELECT op, no_idx FROM test_tpcpatch_mest ORDER BY op;
SELECT * FROM test_tpcpatch_mest
WHERE no_idx IS DISTINCT FROM rtree_equi_idx OR NOT rtree_equi_probe OR
  no_idx IS DISTINCT FROM rtree_seg_idx OR NOT rtree_seg_probe OR
  no_idx IS DISTINCT FROM rtree_merge_idx OR NOT rtree_merge_probe OR
  no_idx IS DISTINCT FROM rtree_adapt_idx OR NOT rtree_adapt_probe OR
  no_idx IS DISTINCT FROM quadtree_equi_idx OR NOT quadtree_equi_probe OR
  no_idx IS DISTINCT FROM quadtree_seg_idx OR NOT quadtree_seg_probe OR
  no_idx IS DISTINCT FROM quadtree_merge_idx OR NOT quadtree_merge_probe OR
  no_idx IS DISTINCT FROM quadtree_adapt_idx OR NOT quadtree_adapt_probe OR
  no_idx IS DISTINCT FROM kdtree_equi_idx OR NOT kdtree_equi_probe OR
  no_idx IS DISTINCT FROM kdtree_seg_idx OR NOT kdtree_seg_probe OR
  no_idx IS DISTINCT FROM kdtree_merge_idx OR NOT kdtree_merge_probe OR
  no_idx IS DISTINCT FROM kdtree_adapt_idx OR NOT kdtree_adapt_probe
ORDER BY op;

DROP TABLE test_tpcpatch_mest;
DROP TABLE tbl_tpcpatch_idx;

DROP FUNCTION pg_temp.test_probes_index(text);

-------------------------------------------------------------------------------
