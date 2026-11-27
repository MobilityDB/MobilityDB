-------------------------------------------------------------------------------
-- The multi-entry R-tree mgist, quad-tree and k-d tree mspgist indexes of a
-- floatspanset, keyed by the spans of its components, one per split (equi, seg,
-- merge, adapt). Under each index every operator the class holds, with each right
-- argument it declares, answers the rows a sequential scan gives, from a plan that
-- probes the index.
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

DROP TABLE IF EXISTS tbl_floatspanset_idx;
CREATE TEMP TABLE tbl_floatspanset_idx AS TABLE tbl_floatspanset;
ANALYZE tbl_floatspanset_idx;

DROP TABLE IF EXISTS test_floatspanset_mest;
CREATE TEMP TABLE test_floatspanset_mest(op text, no_idx text,
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

INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('<< float', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('<< floatspan', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('<< floatspanset', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('&< float', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('&< floatspan', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('&< floatspanset', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('&& floatspan', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('&& floatspanset', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('&> float', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('&> floatspan', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('&> floatspanset', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('>> float', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('>> floatspan', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('>> floatspanset', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('@> float', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('@> floatspan', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('@> floatspanset', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('<@ floatspan', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('<@ floatspanset', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('-|- float', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('-|- floatspan', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('-|- floatspanset', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f)::text);
INSERT INTO test_floatspanset_mest(op, no_idx) VALUES ('= floatspanset', (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f)::text);

-------------------------------------------------------------------------------

CREATE INDEX tbl_floatspanset_idx_rtree_equi ON tbl_floatspanset_idx
  USING mgist(f floatspanset_mrtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f')
WHERE op = '<< float';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspan';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspanset';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f')
WHERE op = '&< float';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspan';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspanset';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspan';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspanset';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f')
WHERE op = '&> float';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspan';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspanset';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f')
WHERE op = '>> float';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspan';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspanset';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f')
WHERE op = '@> float';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspan';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspanset';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspan';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspanset';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- float';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspan';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspanset';
UPDATE test_floatspanset_mest SET rtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f)::text,
  rtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f')
WHERE op = '= floatspanset';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_floatspanset_idx_rtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_floatspanset_idx_rtree_seg ON tbl_floatspanset_idx
  USING mgist(f floatspanset_mrtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f')
WHERE op = '<< float';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspan';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspanset';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f')
WHERE op = '&< float';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspan';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspanset';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspan';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspanset';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f')
WHERE op = '&> float';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspan';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspanset';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f')
WHERE op = '>> float';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspan';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspanset';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f')
WHERE op = '@> float';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspan';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspanset';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspan';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspanset';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- float';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspan';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspanset';
UPDATE test_floatspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f')
WHERE op = '= floatspanset';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_floatspanset_idx_rtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_floatspanset_idx_rtree_merge ON tbl_floatspanset_idx
  USING mgist(f floatspanset_mrtree_mergesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f')
WHERE op = '<< float';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspan';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspanset';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f')
WHERE op = '&< float';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspan';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspanset';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspan';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspanset';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f')
WHERE op = '&> float';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspan';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspanset';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f')
WHERE op = '>> float';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspan';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspanset';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f')
WHERE op = '@> float';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspan';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspanset';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspan';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspanset';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- float';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspan';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspanset';
UPDATE test_floatspanset_mest SET rtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f)::text,
  rtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f')
WHERE op = '= floatspanset';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_floatspanset_idx_rtree_merge;

-------------------------------------------------------------------------------

CREATE INDEX tbl_floatspanset_idx_rtree_adapt ON tbl_floatspanset_idx
  USING mgist(f floatspanset_mrtree_adaptsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f')
WHERE op = '<< float';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspan';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspanset';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f')
WHERE op = '&< float';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspan';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspanset';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspan';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspanset';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f')
WHERE op = '&> float';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspan';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspanset';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f')
WHERE op = '>> float';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspan';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspanset';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f')
WHERE op = '@> float';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspan';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspanset';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspan';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspanset';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- float';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspan';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspanset';
UPDATE test_floatspanset_mest SET rtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f)::text,
  rtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f')
WHERE op = '= floatspanset';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_floatspanset_idx_rtree_adapt;

-------------------------------------------------------------------------------

CREATE INDEX tbl_floatspanset_idx_quadtree_equi ON tbl_floatspanset_idx
  USING mspgist(f floatspanset_mquadtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f')
WHERE op = '<< float';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspan';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspanset';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f')
WHERE op = '&< float';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspan';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspanset';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspan';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspanset';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f')
WHERE op = '&> float';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspan';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspanset';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f')
WHERE op = '>> float';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspan';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspanset';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f')
WHERE op = '@> float';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspan';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspanset';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspan';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspanset';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- float';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspan';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspanset';
UPDATE test_floatspanset_mest SET quadtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f)::text,
  quadtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f')
WHERE op = '= floatspanset';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_floatspanset_idx_quadtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_floatspanset_idx_quadtree_seg ON tbl_floatspanset_idx
  USING mspgist(f floatspanset_mquadtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f')
WHERE op = '<< float';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspan';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspanset';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f')
WHERE op = '&< float';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspan';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspanset';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspan';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspanset';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f')
WHERE op = '&> float';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspan';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspanset';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f')
WHERE op = '>> float';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspan';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspanset';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f')
WHERE op = '@> float';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspan';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspanset';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspan';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspanset';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- float';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspan';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspanset';
UPDATE test_floatspanset_mest SET quadtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f)::text,
  quadtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f')
WHERE op = '= floatspanset';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_floatspanset_idx_quadtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_floatspanset_idx_quadtree_merge ON tbl_floatspanset_idx
  USING mspgist(f floatspanset_mquadtree_mergesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f')
WHERE op = '<< float';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspan';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspanset';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f')
WHERE op = '&< float';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspan';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspanset';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspan';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspanset';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f')
WHERE op = '&> float';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspan';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspanset';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f')
WHERE op = '>> float';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspan';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspanset';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f')
WHERE op = '@> float';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspan';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspanset';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspan';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspanset';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- float';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspan';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspanset';
UPDATE test_floatspanset_mest SET quadtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f)::text,
  quadtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f')
WHERE op = '= floatspanset';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_floatspanset_idx_quadtree_merge;

-------------------------------------------------------------------------------

CREATE INDEX tbl_floatspanset_idx_quadtree_adapt ON tbl_floatspanset_idx
  USING mspgist(f floatspanset_mquadtree_adaptsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f')
WHERE op = '<< float';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspan';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspanset';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f')
WHERE op = '&< float';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspan';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspanset';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspan';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspanset';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f')
WHERE op = '&> float';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspan';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspanset';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f')
WHERE op = '>> float';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspan';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspanset';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f')
WHERE op = '@> float';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspan';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspanset';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspan';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspanset';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- float';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspan';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspanset';
UPDATE test_floatspanset_mest SET quadtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f)::text,
  quadtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f')
WHERE op = '= floatspanset';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_floatspanset_idx_quadtree_adapt;

-------------------------------------------------------------------------------

CREATE INDEX tbl_floatspanset_idx_kdtree_equi ON tbl_floatspanset_idx
  USING mspgist(f floatspanset_mkdtree_equisplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f')
WHERE op = '<< float';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspan';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspanset';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f')
WHERE op = '&< float';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspan';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspanset';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspan';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspanset';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f')
WHERE op = '&> float';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspan';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspanset';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f')
WHERE op = '>> float';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspan';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspanset';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f')
WHERE op = '@> float';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspan';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspanset';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspan';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspanset';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- float';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspan';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspanset';
UPDATE test_floatspanset_mest SET kdtree_equi_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f)::text,
  kdtree_equi_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f')
WHERE op = '= floatspanset';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_floatspanset_idx_kdtree_equi;

-------------------------------------------------------------------------------

CREATE INDEX tbl_floatspanset_idx_kdtree_seg ON tbl_floatspanset_idx
  USING mspgist(f floatspanset_mkdtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f')
WHERE op = '<< float';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspan';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspanset';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f')
WHERE op = '&< float';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspan';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspanset';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspan';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspanset';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f')
WHERE op = '&> float';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspan';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspanset';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f')
WHERE op = '>> float';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspan';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspanset';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f')
WHERE op = '@> float';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspan';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspanset';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspan';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspanset';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- float';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspan';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspanset';
UPDATE test_floatspanset_mest SET kdtree_seg_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f)::text,
  kdtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f')
WHERE op = '= floatspanset';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_floatspanset_idx_kdtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_floatspanset_idx_kdtree_merge ON tbl_floatspanset_idx
  USING mspgist(f floatspanset_mkdtree_mergesplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f')
WHERE op = '<< float';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspan';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspanset';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f')
WHERE op = '&< float';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspan';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspanset';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspan';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspanset';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f')
WHERE op = '&> float';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspan';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspanset';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f')
WHERE op = '>> float';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspan';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspanset';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f')
WHERE op = '@> float';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspan';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspanset';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspan';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspanset';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- float';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspan';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspanset';
UPDATE test_floatspanset_mest SET kdtree_merge_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f)::text,
  kdtree_merge_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f')
WHERE op = '= floatspanset';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_floatspanset_idx_kdtree_merge;

-------------------------------------------------------------------------------

CREATE INDEX tbl_floatspanset_idx_kdtree_adapt ON tbl_floatspanset_idx
  USING mspgist(f floatspanset_mkdtree_adaptsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f << t2.f')
WHERE op = '<< float';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspan';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f << t2.f')
WHERE op = '<< floatspanset';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &< t2.f')
WHERE op = '&< float';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspan';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &< t2.f')
WHERE op = '&< floatspanset';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspan';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f && t2.f')
WHERE op = '&& floatspanset';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f &> t2.f')
WHERE op = '&> float';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspan';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f &> t2.f')
WHERE op = '&> floatspanset';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f >> t2.f')
WHERE op = '>> float';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspan';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f >> t2.f')
WHERE op = '>> floatspanset';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f @> t2.f')
WHERE op = '@> float';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspan';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f @> t2.f')
WHERE op = '@> floatspanset';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspan';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f')
WHERE op = '<@ floatspanset';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_float t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- float';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspan t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspan';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f')
WHERE op = '-|- floatspanset';
UPDATE test_floatspanset_mest SET kdtree_adapt_idx = (SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f)::text,
  kdtree_adapt_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_floatspanset_idx t1, tbl_floatspanset t2 WHERE t1.f = t2.f')
WHERE op = '= floatspanset';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_floatspanset_idx_kdtree_adapt;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_floatspanset_idx;
SELECT COUNT(*) FROM test_floatspanset_mest;
SELECT op, no_idx FROM test_floatspanset_mest ORDER BY op;
SELECT * FROM test_floatspanset_mest
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

DROP TABLE test_floatspanset_mest;
DROP TABLE tbl_floatspanset_idx;

DROP FUNCTION pg_temp.test_probes_index(text);

-------------------------------------------------------------------------------
