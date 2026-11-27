-------------------------------------------------------------------------------
-- The control of the multi-entry tests for a tbool: the class of the segment split
-- of the R-tree and its control, the same class whose consistent method is that of
-- the single-entry class, which reads a key as the whole value. The rows printed
-- are those where an index differs from the sequential scan: none of the class,
-- and some of the control, which the comparison of every multi-entry test notices.
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

DROP TABLE IF EXISTS tbl_tbool_idx;
CREATE TEMP TABLE tbl_tbool_idx AS TABLE tbl_tbool;
ANALYZE tbl_tbool_idx;

CREATE OPERATOR CLASS tbool_mrtree_control_ops
  FOR TYPE tbool USING mgist AS
  STORAGE tstzspan,
  -- same
  OPERATOR  6    ~= (tbool, tstzspan),
  OPERATOR  6    ~= (tbool, tbool),
  -- contained by
  OPERATOR  8    <@ (tbool, tstzspan),
  OPERATOR  8    <@ (tbool, tbool),
  -- overlaps or before
  OPERATOR  28    &<# (tbool, tstzspan),
  OPERATOR  28    &<# (tbool, tbool),
  -- strictly before
  OPERATOR  29    <<# (tbool, tstzspan),
  OPERATOR  29    <<# (tbool, tbool),
  -- strictly after
  OPERATOR  30    #>> (tbool, tstzspan),
  OPERATOR  30    #>> (tbool, tbool),
  -- overlaps or after
  OPERATOR  31    #&> (tbool, tstzspan),
  OPERATOR  31    #&> (tbool, tbool),
  -- ever overlaps
  OPERATOR  36    ?&& (tbool, tstzspan),
  -- always overlaps
  OPERATOR  37    %&& (tbool, tstzspan),
  -- functions
  FUNCTION  1 tbool_gist_consistent(internal, tbool, smallint, oid, internal),
  FUNCTION  2 span_gist_union(internal, internal),
  FUNCTION  3 mest_gist_compress(internal),
  FUNCTION  5 span_gist_penalty(internal, internal, internal),
  FUNCTION  6 span_gist_picksplit(internal, internal),
  FUNCTION  7 span_gist_same(tstzspan, tstzspan, internal),
  FUNCTION 10 mest_count_options(internal),
  FUNCTION 13 temporal_mest_segsplit(internal, internal, internal);

DROP TABLE IF EXISTS test_tbool_mest;
CREATE TEMP TABLE test_tbool_mest(op text, no_idx text,
  rtree_seg_idx text, rtree_seg_probe boolean,
  control_idx text, control_probe boolean);

INSERT INTO test_tbool_mest(op, no_idx) VALUES ('~= tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_tbool_mest(op, no_idx) VALUES ('~= tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_tbool_mest(op, no_idx) VALUES ('<@ tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text);
INSERT INTO test_tbool_mest(op, no_idx) VALUES ('<@ tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_tbool_mest(op, no_idx) VALUES ('&<# tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_tbool_mest(op, no_idx) VALUES ('&<# tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_tbool_mest(op, no_idx) VALUES ('<<# tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_tbool_mest(op, no_idx) VALUES ('<<# tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_tbool_mest(op, no_idx) VALUES ('#>> tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_tbool_mest(op, no_idx) VALUES ('#>> tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_tbool_mest(op, no_idx) VALUES ('#&> tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_tbool_mest(op, no_idx) VALUES ('#&> tbool', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_tbool_mest(op, no_idx) VALUES ('?&& tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp ?&& t2.t)::text);
INSERT INTO test_tbool_mest(op, no_idx) VALUES ('%&& tstzspan', (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp %&& t2.t)::text);

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_idx_rtree_seg ON tbl_tbool_idx
  USING mgist(temp tbool_mrtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbool_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tbool_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tbool';
UPDATE test_tbool_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tbool_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tbool';
UPDATE test_tbool_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tbool_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tbool';
UPDATE test_tbool_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tbool_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tbool';
UPDATE test_tbool_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tbool_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tbool';
UPDATE test_tbool_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tbool_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tbool';
UPDATE test_tbool_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp ?&& t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp ?&& t2.t')
WHERE op = '?&& tstzspan';
UPDATE test_tbool_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp %&& t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp %&& t2.t')
WHERE op = '%&& tstzspan';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbool_idx_rtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_idx_control ON tbl_tbool_idx
  USING mgist(temp tbool_mrtree_control_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tbool_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tbool_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp ~= t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tbool';
UPDATE test_tbool_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tbool_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp <@ t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tbool';
UPDATE test_tbool_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tbool_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp &<# t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tbool';
UPDATE test_tbool_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tbool_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp <<# t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tbool';
UPDATE test_tbool_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tbool_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp #>> t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tbool';
UPDATE test_tbool_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tbool_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp #&> t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tbool t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tbool';
UPDATE test_tbool_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp ?&& t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp ?&& t2.t')
WHERE op = '?&& tstzspan';
UPDATE test_tbool_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp %&& t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tbool_idx t1, tbl_tstzspan t2 WHERE t1.temp %&& t2.t')
WHERE op = '%&& tstzspan';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tbool_idx_control;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool_idx;
SELECT COUNT(*) FROM test_tbool_mest;
SELECT op, no_idx FROM test_tbool_mest ORDER BY op;
SELECT * FROM test_tbool_mest
WHERE no_idx IS DISTINCT FROM rtree_seg_idx OR NOT rtree_seg_probe OR
  no_idx IS DISTINCT FROM control_idx OR NOT control_probe
ORDER BY op;

DROP TABLE test_tbool_mest;
DROP TABLE tbl_tbool_idx;
DROP OPERATOR FAMILY tbool_mrtree_control_ops USING mgist;

DROP FUNCTION pg_temp.test_probes_index(text);

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- The control of the multi-entry tests for a tgeompoint: the class of the segment
-- split of the R-tree and its control, the same class whose consistent method is
-- that of the single-entry class, which reads a key as the whole value. The rows
-- printed are those where an index differs from the sequential scan: none of the
-- class, and some of the control, which the comparison of every multi-entry test
-- notices.
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

DROP TABLE IF EXISTS tbl_tgeompoint_idx;
CREATE TEMP TABLE tbl_tgeompoint_idx AS TABLE tbl_tgeompoint;
ANALYZE tbl_tgeompoint_idx;

CREATE OPERATOR CLASS tgeompoint_mrtree_control_ops
  FOR TYPE tgeompoint USING mgist AS
  STORAGE stbox,
  -- strictly left
  OPERATOR  1    << (tgeompoint, stbox),
  OPERATOR  1    << (tgeompoint, tgeompoint),
  -- overlaps or left
  OPERATOR  2    &< (tgeompoint, stbox),
  OPERATOR  2    &< (tgeompoint, tgeompoint),
  -- overlaps
  OPERATOR  3    && (tgeompoint, tstzspan),
  OPERATOR  3    && (tgeompoint, stbox),
  OPERATOR  3    && (tgeompoint, tgeompoint),
  -- overlaps or right
  OPERATOR  4    &> (tgeompoint, stbox),
  OPERATOR  4    &> (tgeompoint, tgeompoint),
  -- strictly right
  OPERATOR  5    >> (tgeompoint, stbox),
  OPERATOR  5    >> (tgeompoint, tgeompoint),
  -- same
  OPERATOR  6    ~= (tgeompoint, tstzspan),
  OPERATOR  6    ~= (tgeompoint, stbox),
  OPERATOR  6    ~= (tgeompoint, tgeompoint),
  -- contains
  OPERATOR  7    @> (tgeompoint, tstzspan),
  OPERATOR  7    @> (tgeompoint, stbox),
  OPERATOR  7    @> (tgeompoint, tgeompoint),
  -- contained by
  OPERATOR  8    <@ (tgeompoint, tstzspan),
  OPERATOR  8    <@ (tgeompoint, stbox),
  OPERATOR  8    <@ (tgeompoint, tgeompoint),
  -- overlaps or below
  OPERATOR  9    &<| (tgeompoint, stbox),
  OPERATOR  9    &<| (tgeompoint, tgeompoint),
  -- strictly below
  OPERATOR  10    <<| (tgeompoint, stbox),
  OPERATOR  10    <<| (tgeompoint, tgeompoint),
  -- strictly above
  OPERATOR  11    |>> (tgeompoint, stbox),
  OPERATOR  11    |>> (tgeompoint, tgeompoint),
  -- overlaps or above
  OPERATOR  12    |&> (tgeompoint, stbox),
  OPERATOR  12    |&> (tgeompoint, tgeompoint),
  -- adjacent
  OPERATOR  17    -|- (tgeompoint, tstzspan),
  OPERATOR  17    -|- (tgeompoint, stbox),
  OPERATOR  17    -|- (tgeompoint, tgeompoint),
  -- nearest approach distance
  OPERATOR  25    |=| (tgeompoint, stbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeompoint, tgeompoint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeompoint, geometry) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tgeompoint, tstzspan),
  OPERATOR  28    &<# (tgeompoint, stbox),
  OPERATOR  28    &<# (tgeompoint, tgeompoint),
  -- strictly before
  OPERATOR  29    <<# (tgeompoint, tstzspan),
  OPERATOR  29    <<# (tgeompoint, stbox),
  OPERATOR  29    <<# (tgeompoint, tgeompoint),
  -- strictly after
  OPERATOR  30    #>> (tgeompoint, tstzspan),
  OPERATOR  30    #>> (tgeompoint, stbox),
  OPERATOR  30    #>> (tgeompoint, tgeompoint),
  -- overlaps or after
  OPERATOR  31    #&> (tgeompoint, tstzspan),
  OPERATOR  31    #&> (tgeompoint, stbox),
  OPERATOR  31    #&> (tgeompoint, tgeompoint),
  -- overlaps or front
  OPERATOR  32    &</ (tgeompoint, stbox),
  OPERATOR  32    &</ (tgeompoint, tgeompoint),
  -- strictly front
  OPERATOR  33    <</ (tgeompoint, stbox),
  OPERATOR  33    <</ (tgeompoint, tgeompoint),
  -- strictly back
  OPERATOR  34    />> (tgeompoint, stbox),
  OPERATOR  34    />> (tgeompoint, tgeompoint),
  -- overlaps or back
  OPERATOR  35    /&> (tgeompoint, stbox),
  OPERATOR  35    /&> (tgeompoint, tgeompoint),
  -- ever overlaps
  OPERATOR  36    ?&& (tgeompoint, stbox),
  -- always overlaps
  OPERATOR  37    %&& (tgeompoint, stbox),
  -- functions
  FUNCTION  1 tgeompoint_gist_consistent(internal, tgeompoint, smallint, oid, internal),
  FUNCTION  2 stbox_gist_union(internal, internal),
  FUNCTION  3 mest_gist_compress(internal),
  FUNCTION  5 stbox_gist_penalty(internal, internal, internal),
  FUNCTION  6 stbox_gist_picksplit(internal, internal),
  FUNCTION  7 stbox_gist_same(stbox, stbox, internal),
  FUNCTION  8 tgeompoint_gist_distance(internal, tgeompoint, smallint, oid, internal),
  FUNCTION 10 mest_count_options(internal),
  FUNCTION 13 tgeo_mest_segsplit(internal, internal, internal);

DROP TABLE IF EXISTS test_tgeompoint_mest;
CREATE TEMP TABLE test_tgeompoint_mest(op text, no_idx text,
  rtree_seg_idx text, rtree_seg_probe boolean,
  control_idx text, control_probe boolean);

INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('<< stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('<< tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('&< stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('&< tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('&& tstzspan', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('&& stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('&& tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('&> stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('&> tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('>> stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('>> tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('~= tstzspan', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('~= stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('~= tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('@> tstzspan', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('@> stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('@> tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp @> t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('<@ tstzspan', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('<@ stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('<@ tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('&<| stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('&<| tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('<<| stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('<<| tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('|>> stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('|>> tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('|&> stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('|&> tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('-|- tstzspan', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('-|- stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('-|- tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('&<# tstzspan', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('&<# stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &<# t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('&<# tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('<<# tstzspan', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('<<# stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <<# t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('<<# tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('#>> tstzspan', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('#>> stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp #>> t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('#>> tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('#&> tstzspan', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('#&> stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp #&> t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('#&> tgeompoint', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('?&& stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp ?&& t2.b)::text);
INSERT INTO test_tgeompoint_mest(op, no_idx) VALUES ('%&& stbox', (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp %&& t2.b)::text);

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint_idx_rtree_seg ON tbl_tgeompoint_idx
  USING mgist(temp tgeompoint_mrtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp << t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp && t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp && t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp @> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &<# t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <<# t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp #>> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp #&> t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tgeompoint';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp ?&& t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tgeompoint_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp %&& t2.b)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tgeompoint_idx_rtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint_idx_control ON tbl_tgeompoint_idx
  USING mgist(temp tgeompoint_mrtree_control_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp << t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp')
WHERE op = '<< tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &< t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp && t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp && t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp && t2.temp')
WHERE op = '&& tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &> t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp >> t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp ~= t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp ~= t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp @> t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp @> t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <@ t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <@ t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &<| t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <<| t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp |>> t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp |&> t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp -|- t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp -|- t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &<# t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <<# t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp #>> t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tstzspan t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp #&> t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> tgeompoint';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp ?&& t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp ?&& t2.b')
WHERE op = '?&& stbox';
UPDATE test_tgeompoint_mest SET control_idx = (SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp %&& t2.b)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_tgeompoint_idx t1, tbl_stbox t2 WHERE t1.temp %&& t2.b')
WHERE op = '%&& stbox';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_tgeompoint_idx_control;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeompoint_idx;
SELECT COUNT(*) FROM test_tgeompoint_mest;
SELECT op, no_idx FROM test_tgeompoint_mest ORDER BY op;
SELECT * FROM test_tgeompoint_mest
WHERE no_idx IS DISTINCT FROM rtree_seg_idx OR NOT rtree_seg_probe OR
  no_idx IS DISTINCT FROM control_idx OR NOT control_probe
ORDER BY op;

DROP TABLE test_tgeompoint_mest;
DROP TABLE tbl_tgeompoint_idx;
DROP OPERATOR FAMILY tgeompoint_mrtree_control_ops USING mgist;

DROP FUNCTION pg_temp.test_probes_index(text);

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- The control of the multi-entry tests for a intspanset: the class of the segment
-- split of the R-tree and its control, the same class whose consistent method is
-- that of the single-entry class, which reads a key as the whole value. The rows
-- printed are those where an index differs from the sequential scan: none of the
-- class, and some of the control, which the comparison of every multi-entry test
-- notices.
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

DROP TABLE IF EXISTS tbl_intspanset_idx;
CREATE TEMP TABLE tbl_intspanset_idx AS TABLE tbl_intspanset;
ANALYZE tbl_intspanset_idx;

CREATE OPERATOR CLASS intspanset_mrtree_control_ops
  FOR TYPE intspanset USING mgist AS
  STORAGE intspan,
  -- strictly left
  OPERATOR  1     << (intspanset, integer),
  OPERATOR  1     << (intspanset, intspan),
  OPERATOR  1     << (intspanset, intspanset),
  -- overlaps or left
  OPERATOR  2     &< (intspanset, integer),
  OPERATOR  2     &< (intspanset, intspan),
  OPERATOR  2     &< (intspanset, intspanset),
  -- overlaps
  OPERATOR  3     && (intspanset, intspan),
  OPERATOR  3     && (intspanset, intspanset),
  -- overlaps or right
  OPERATOR  4     &> (intspanset, integer),
  OPERATOR  4     &> (intspanset, intspan),
  OPERATOR  4     &> (intspanset, intspanset),
  -- strictly right
  OPERATOR  5     >> (intspanset, integer),
  OPERATOR  5     >> (intspanset, intspan),
  OPERATOR  5     >> (intspanset, intspanset),
  -- contains
  OPERATOR  7     @> (intspanset, integer),
  OPERATOR  7     @> (intspanset, intspan),
  OPERATOR  7     @> (intspanset, intspanset),
  -- contained by
  OPERATOR  8     <@ (intspanset, intspan),
  OPERATOR  8     <@ (intspanset, intspanset),
  -- adjacent
  OPERATOR  17    -|- (intspanset, integer),
  OPERATOR  17    -|- (intspanset, intspan),
  OPERATOR  17    -|- (intspanset, intspanset),
  -- equals
  OPERATOR  18    = (intspanset, intspanset),
  -- nearest approach distance
  OPERATOR  25    <-> (intspanset, integer) FOR ORDER BY pg_catalog.integer_ops,
  OPERATOR  25    <-> (intspanset, intspan) FOR ORDER BY pg_catalog.integer_ops,
  OPERATOR  25    <-> (intspanset, intspanset) FOR ORDER BY pg_catalog.integer_ops,
  -- functions
  FUNCTION  1 span_gist_consistent(internal, intspanset, smallint, oid, internal),
  FUNCTION  2 span_mgist_union(internal, internal),
  FUNCTION  3 mest_gist_compress(internal),
  FUNCTION  5 span_mgist_penalty(internal, internal, internal),
  FUNCTION  6 span_mgist_picksplit(internal, internal),
  FUNCTION  7 span_mgist_same(intspan, intspan, internal),
  FUNCTION  8 span_mgist_distance(internal, intspan, smallint, oid, internal),
  FUNCTION 10 mest_count_options(internal),
  FUNCTION 13 spanset_mest_segsplit(internal, internal, internal);

DROP TABLE IF EXISTS test_intspanset_mest;
CREATE TEMP TABLE test_intspanset_mest(op text, no_idx text,
  rtree_seg_idx text, rtree_seg_probe boolean,
  control_idx text, control_probe boolean);

INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('<< integer', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i << t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('<< intspan', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i << t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('<< intspanset', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i << t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('&< integer', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i &< t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('&< intspan', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i &< t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('&< intspanset', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i &< t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('&& intspan', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i && t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('&& intspanset', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i && t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('&> integer', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i &> t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('&> intspan', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i &> t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('&> intspanset', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i &> t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('>> integer', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i >> t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('>> intspan', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i >> t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('>> intspanset', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i >> t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('@> integer', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i @> t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('@> intspan', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i @> t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('@> intspanset', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i @> t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('<@ intspan', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i <@ t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('<@ intspanset', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i <@ t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('-|- integer', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i -|- t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('-|- intspan', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i -|- t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('-|- intspanset', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i -|- t2.i)::text);
INSERT INTO test_intspanset_mest(op, no_idx) VALUES ('= intspanset', (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i = t2.i)::text);

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspanset_idx_rtree_seg ON tbl_intspanset_idx
  USING mgist(i intspanset_mrtree_segsplit_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i << t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i << t2.i')
WHERE op = '<< integer';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i << t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i << t2.i')
WHERE op = '<< intspan';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i << t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i << t2.i')
WHERE op = '<< intspanset';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i &< t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i &< t2.i')
WHERE op = '&< integer';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i &< t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i &< t2.i')
WHERE op = '&< intspan';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i &< t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i &< t2.i')
WHERE op = '&< intspanset';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i && t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i && t2.i')
WHERE op = '&& intspan';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i && t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i && t2.i')
WHERE op = '&& intspanset';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i &> t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i &> t2.i')
WHERE op = '&> integer';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i &> t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i &> t2.i')
WHERE op = '&> intspan';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i &> t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i &> t2.i')
WHERE op = '&> intspanset';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i >> t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i >> t2.i')
WHERE op = '>> integer';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i >> t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i >> t2.i')
WHERE op = '>> intspan';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i >> t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i >> t2.i')
WHERE op = '>> intspanset';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i @> t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i @> t2.i')
WHERE op = '@> integer';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i @> t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i @> t2.i')
WHERE op = '@> intspan';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i @> t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i @> t2.i')
WHERE op = '@> intspanset';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i <@ t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i <@ t2.i')
WHERE op = '<@ intspan';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i <@ t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i <@ t2.i')
WHERE op = '<@ intspanset';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i -|- t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i -|- t2.i')
WHERE op = '-|- integer';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i -|- t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i -|- t2.i')
WHERE op = '-|- intspan';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i -|- t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i -|- t2.i')
WHERE op = '-|- intspanset';
UPDATE test_intspanset_mest SET rtree_seg_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i = t2.i)::text,
  rtree_seg_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i = t2.i')
WHERE op = '= intspanset';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_intspanset_idx_rtree_seg;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspanset_idx_control ON tbl_intspanset_idx
  USING mgist(i intspanset_mrtree_control_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i << t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i << t2.i')
WHERE op = '<< integer';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i << t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i << t2.i')
WHERE op = '<< intspan';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i << t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i << t2.i')
WHERE op = '<< intspanset';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i &< t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i &< t2.i')
WHERE op = '&< integer';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i &< t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i &< t2.i')
WHERE op = '&< intspan';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i &< t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i &< t2.i')
WHERE op = '&< intspanset';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i && t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i && t2.i')
WHERE op = '&& intspan';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i && t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i && t2.i')
WHERE op = '&& intspanset';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i &> t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i &> t2.i')
WHERE op = '&> integer';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i &> t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i &> t2.i')
WHERE op = '&> intspan';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i &> t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i &> t2.i')
WHERE op = '&> intspanset';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i >> t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i >> t2.i')
WHERE op = '>> integer';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i >> t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i >> t2.i')
WHERE op = '>> intspan';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i >> t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i >> t2.i')
WHERE op = '>> intspanset';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i @> t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i @> t2.i')
WHERE op = '@> integer';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i @> t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i @> t2.i')
WHERE op = '@> intspan';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i @> t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i @> t2.i')
WHERE op = '@> intspanset';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i <@ t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i <@ t2.i')
WHERE op = '<@ intspan';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i <@ t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i <@ t2.i')
WHERE op = '<@ intspanset';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i -|- t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_int t2 WHERE t1.i -|- t2.i')
WHERE op = '-|- integer';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i -|- t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspan t2 WHERE t1.i -|- t2.i')
WHERE op = '-|- intspan';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i -|- t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i -|- t2.i')
WHERE op = '-|- intspanset';
UPDATE test_intspanset_mest SET control_idx = (SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i = t2.i)::text,
  control_probe = pg_temp.test_probes_index('SELECT COUNT(*) FROM tbl_intspanset_idx t1, tbl_intspanset t2 WHERE t1.i = t2.i')
WHERE op = '= intspanset';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_intspanset_idx_control;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_intspanset_idx;
SELECT COUNT(*) FROM test_intspanset_mest;
SELECT op, no_idx FROM test_intspanset_mest ORDER BY op;
SELECT * FROM test_intspanset_mest
WHERE no_idx IS DISTINCT FROM rtree_seg_idx OR NOT rtree_seg_probe OR
  no_idx IS DISTINCT FROM control_idx OR NOT control_probe
ORDER BY op;

DROP TABLE test_intspanset_mest;
DROP TABLE tbl_intspanset_idx;
DROP OPERATOR FAMILY intspanset_mrtree_control_ops USING mgist;

DROP FUNCTION pg_temp.test_probes_index(text);

-------------------------------------------------------------------------------
